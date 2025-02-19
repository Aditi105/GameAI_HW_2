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
        this->setRadius(5.f);
        this->setFillColor(sf::Color(0, 0, 255, 255));
        this->setPosition(-100, -100); // initially offscreen
        this->setOrigin(5.f, 5.f);
    }

    void draw(sf::RenderWindow* window) {
        window->draw(*this);
    }

    void drop(const sf::Vector2f& pos) {
        this->setPosition(pos);
    }

private:
    int id;
};

// -----------------------------------------------------------------
// Main Application
// -----------------------------------------------------------------
int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Part 2");

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
    // Scale the sprite three times larger.
    boidSprite.setScale(1.5f, 1.5f);

    // Initialize the boid's kinematic state.
    Kinematic character;
    character.position = sf::Vector2f(400.f, 300.f);
    character.velocity = sf::Vector2f(0.f, 0.f);
    character.orientation = 0.f; // radians; 0 means facing right
    character.rotation = 0.f;

    // Initialize the target kinematic.
    Kinematic targetKinematic;
    targetKinematic.position = character.position;
    targetKinematic.velocity = sf::Vector2f(0.f, 0.f);
    targetKinematic.orientation = character.orientation;
    targetKinematic.rotation = 0.f;

    // Create Arrive and Align behaviors.
    ArriveBehavior arrive(300.f,     // maxAcceleration
                          150.f,     // maxSpeed
                          5.f,       // targetRadius: within 5 pixels, consider arrived
                          100.f,     // slowRadius: start decelerating within 100 pixels
                          0.1f);     // timeToTarget

    AlignBehavior align(5.f,         // maxAngularAcceleration
                        PI / 4.f,    // maxRotation (45 degrees per second)
                        0.05f,       // satisfactionRadius: if within 0.05 radians, no rotation
                        0.5f,        // decelerationRadius: rotate slower when within 0.5 radians
                        0.1f);       // timeToTarget

    sf::Clock clock;

    // The target position is updated on mouse clicks.
    sf::Vector2f targetPos = character.position;

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
    // Main loop.
    // -----------------------------------------------------------------
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            // Update target position on left mouse click.
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                targetPos = sf::Vector2f(static_cast<float>(event.mouseButton.x),
                                         static_cast<float>(event.mouseButton.y));
            }
        }

        float deltaTime = clock.restart().asSeconds();

        // Update the target kinematic.
        targetKinematic.position = targetPos;
        sf::Vector2f toTarget = targetPos - character.position;
        float distance = length(toTarget);
        if (distance > 0.001f)
            targetKinematic.orientation = std::atan2(toTarget.y, toTarget.x);
        else
            targetKinematic.orientation = character.orientation;

        // Get steering outputs from Arrive and Align behaviors.
        SteeringOutput arriveSteering = arrive.getSteering(character, targetKinematic, deltaTime);
        SteeringOutput alignSteering = align.getSteering(character, targetKinematic, deltaTime);

        // Update character's linear movement using Arrive.
        character.velocity += arriveSteering.linear * deltaTime;
        character.position += character.velocity * deltaTime;

        // If within target threshold and nearly stopped, snap to target and stop angular updates.
        if (distance < 5.f && length(character.velocity) < 1.f) {
            character.velocity = sf::Vector2f(0.f, 0.f);
            character.position = targetPos;
            // Prevent further rotation if arrived.
            character.rotation = 0.f;
        } else {
            // Update character's angular movement using Align.
            character.rotation += alignSteering.angular * deltaTime;
            character.orientation += character.rotation * deltaTime;
            character.orientation = mapToRange(character.orientation);
        }

        // Update the sprite's position and rotation (convert radians to degrees).
        boidSprite.setPosition(character.position);
        boidSprite.setRotation(character.orientation * 180.f / PI);

        // Update breadcrumb drop timer.
        dropTimer += deltaTime;
        if (dropTimer >= dropInterval) {
            dropTimer = 0.f;
            breadcrumbs[crumbIndex].drop(character.position);
            crumbIndex = (crumbIndex + 1) % maxBreadcrumbs;
        }

        // Rendering.
        window.clear(sf::Color::White);
        // Draw breadcrumbs.
        for (auto& crumb : breadcrumbs) {
            crumb.draw(&window);
        }
        // Draw the boid sprite.
        window.draw(boidSprite);
        window.display();
    }

    return 0;
}
