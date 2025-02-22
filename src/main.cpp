#include <SFML/Graphics.hpp>
#include "Steering.hpp"
#include <vector>
#include <iostream>

// -----------------------------------------------------------------
// Crumb Class: Represents a breadcrumb dropped along the boid's path.
// -----------------------------------------------------------------
class Crumb : public sf::CircleShape {
public:
    Crumb(int id) : id(id) {
        // Set size and color for the breadcrumb.
        setRadius(5.f);
        setFillColor(sf::Color(0, 0, 255, 255));
        setPosition(-100, -100); // initially offscreen
        setOrigin(5.f, 5.f);
    }

    void draw(sf::RenderWindow* window) {
        window->draw(*this);
    }

    void drop(const sf::Vector2f& pos) {
        setPosition(pos);
    }

private:
    int id;
};

// -----------------------------------------------------------------
// Main Application
// -----------------------------------------------------------------
int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Arrive and Align Demo");

    // Load the boid texture.
    sf::Texture boidTexture;
    if (!boidTexture.loadFromFile("./src/boid-sm.png")) {
        std::cerr << "Failed to load boid-sm.png" << std::endl;
        return -1;
    }

    // Create the boid sprite.
    sf::Sprite boidSprite;
    boidSprite.setTexture(boidTexture);
    sf::FloatRect spriteBounds = boidSprite.getLocalBounds();
    boidSprite.setOrigin(spriteBounds.width / 2.f, spriteBounds.height / 2.f);
    // Scale the sprite up by a factor of 4.
    boidSprite.setScale(4.0f, 4.0f);

    // Initialize the boid's kinematic state.
    Kinematic character;
    character.position = sf::Vector2f(400.f, 300.f);
    character.velocity = sf::Vector2f(0.f, 0.f);
    character.orientation = 0.f; // radians; 0 means facing right
    character.rotation = 0.f;

    // Initialize the target kinematic (starts at the character's position).
    Kinematic targetKinematic;
    targetKinematic.position = character.position;
    targetKinematic.velocity = sf::Vector2f(0.f, 0.f);
    targetKinematic.orientation = character.orientation;
    targetKinematic.rotation = 0.f;

    // Create Arrive and Align behaviors.
    // (Increase slowRadius for earlier deceleration; adjust parameters as needed.)
    ArriveBehavior arrive(300.f,     // maxAcceleration
                          150.f,     // maxSpeed
                          5.f,       // targetRadius: within 5 pixels, consider arrived
                          200.f,     // slowRadius: begin deceleration within 200 pixels
                          0.1f);     // timeToTarget

    AlignBehavior align(18.f,         // maxAngularAcceleration
                        PI / 2.f,    // maxRotation (90 degrees per second)
                        0.05f,       // satisfactionRadius: if within 0.05 radians, no rotation needed
                        0.5f,        // decelerationRadius: begin slowing rotation within 0.5 radians
                        0.1f);       // timeToTarget

    sf::Clock clock;

    // The target position is updated on mouse clicks.
    sf::Vector2f targetPos = character.position;

    // Introduce a flag to freeze updates once the boid has arrived.
    bool frozen = false;

    // -----------------------------------------------------------------
    // Breadcrumbs: fixed-length vector (10 breadcrumbs), dropped at intervals.
    // -----------------------------------------------------------------
    const int maxBreadcrumbs = 10;
    std::vector<Crumb> breadcrumbs;
    for (int i = 0; i < maxBreadcrumbs; i++) {
        breadcrumbs.push_back(Crumb(i));
    }
    int crumbIndex = 0;
    float dropTimer = 0.f;
    const float dropInterval = 0.2f; // drop a crumb every 0.2 seconds

    // -----------------------------------------------------------------
    // Main Loop.
    // -----------------------------------------------------------------
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            // On left mouse click, update the target position and unfreeze the boid.
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                targetPos = sf::Vector2f(static_cast<float>(event.mouseButton.x),
                                         static_cast<float>(event.mouseButton.y));
                frozen = false;
            }
        }

        float deltaTime = clock.restart().asSeconds();

        // Update the target kinematic.
        targetKinematic.position = targetPos;
        sf::Vector2f toTarget = targetPos - character.position;
        float distance = vectorLength(toTarget);
        if (distance > 0.001f)
            targetKinematic.orientation = std::atan2(toTarget.y, toTarget.x);
        else
            targetKinematic.orientation = character.orientation;

        if (!frozen) {
            // Get steering outputs from Arrive and Align.
            SteeringOutput arriveSteering = arrive.getSteering(character, targetKinematic, deltaTime);
            SteeringOutput alignSteering = align.getSteering(character, targetKinematic, deltaTime);

            // Update linear movement.
            character.velocity += arriveSteering.linear * deltaTime;
            character.position += character.velocity * deltaTime;

            // Check arrival conditions: within 1 pixel and nearly zero speed.
            bool arrived = (distance < 1.f) && (vectorLength(character.velocity) < 0.1f);
            if (arrived) {
                // Snap exactly to target.
                character.position = targetPos;
                character.velocity = sf::Vector2f(0.f, 0.f);
                character.rotation = 0.f;
                frozen = true;
            } else {
                // Update angular movement.
                character.rotation += alignSteering.angular * deltaTime;
                character.orientation += character.rotation * deltaTime;
                character.orientation = mapToRange(character.orientation);
            }
        } // If frozen, no updates occur.

        // Update sprite's position and rotation (convert radians to degrees).
        boidSprite.setPosition(character.position);
        boidSprite.setRotation(character.orientation * 180.f / PI);

        // Drop breadcrumbs at fixed intervals.
        dropTimer += deltaTime;
        if (dropTimer >= dropInterval) {
            dropTimer = 0.f;
            breadcrumbs[crumbIndex].drop(character.position);
            crumbIndex = (crumbIndex + 1) % maxBreadcrumbs;
        }

        // Render everything.
        window.clear(sf::Color::White);
        for (auto& crumb : breadcrumbs)
            crumb.draw(&window);
        window.draw(boidSprite);
        window.display();
    }

    return 0;
}
