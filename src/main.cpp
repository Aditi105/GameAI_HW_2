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
    // Scale the sprite up by a factor of 4.
    boidSprite.setScale(4.0f, 4.0f);

    // Initialize the boid's kinematic state.
    Kinematic character;
    character.position = sf::Vector2f(400.f, 300.f);
    character.velocity = sf::Vector2f(0.f, 0.f);
    character.orientation = 0.f;
    character.rotation = 0.f;

    // The target kinematic starts at the character's position.
    Kinematic targetKinematic;
    targetKinematic.position = character.position;
    targetKinematic.velocity = sf::Vector2f(0.f, 0.f);
    targetKinematic.orientation = character.orientation;
    targetKinematic.rotation = 0.f;

    // Create Arrive and Align behaviors.
    // ArriveBehavior(maxAcceleration, maxSpeed, targetRadius, slowRadius, timeToTarget)
    ArriveBehavior arrive(
        300.f,    // Maximum linear acceleration (pixels/second^2)
        250.f,    // Maximum speed (pixels/second)
        5.f,      // Target radius: if within 5 pixels, consider arrived (and no further acceleration)
        200.f,    // Slow radius: begin deceleration when within 200 pixels of the target
        0.05f      // Time to target: time over which to achieve the target speed (in seconds)
    );

    // AlignBehavior(maxAngularAcceleration, maxRotation, satisfactionRadius, decelerationRadius, timeToTarget)
    AlignBehavior align(
        18.f,          // Maximum angular acceleration (radians/second^2)
        PI / 1.f,      // Maximum rotation speed (radians/second) - here 90° per second
        0.05f,         // Satisfaction radius: if within 0.05 radians, no further rotation is needed
        0.5f,          // Deceleration radius: begin decelerating rotation when within 0.5 radians
        0.1f           // Time to target: time over which to achieve the target rotation (in seconds)
    );


    sf::Clock clock;
    // The target position is updated on mouse clicks.
    sf::Vector2f targetPos = character.position;
    // Freeze flag: once true, the boid remains at the target.
    bool frozen = false;
    // Store the final orientation when the boid arrives.
    float finalOrientation = character.orientation;

    // -----------------------------------------------------------------
    // Breadcrumbs: fixed-length vector (10 breadcrumbs), dropped at intervals.
    // -----------------------------------------------------------------
    const int maxBreadcrumbs = 150;
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
            // On left mouse click, update target position and unfreeze.
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
            // Get steering outputs.
            SteeringOutput arriveSteering = arrive.getSteering(character, targetKinematic, deltaTime);
            SteeringOutput alignSteering = align.getSteering(character, targetKinematic, deltaTime);

            // Update linear movement.
            character.velocity += arriveSteering.linear * deltaTime;
            character.position += character.velocity * deltaTime;

            // Check arrival conditions: within 1 pixel and speed below 0.1.
            bool arrived = (distance < 1.f) && (vectorLength(character.velocity) < 0.1f);
            if (arrived) {
                character.position = targetPos;
                character.velocity = sf::Vector2f(0.f, 0.f);
                character.rotation = 0.f;
                // Store the final orientation (from the target).
                finalOrientation = targetKinematic.orientation;
                frozen = true;
            } else {
                // Update angular movement.
                character.rotation += alignSteering.angular * deltaTime;
                character.orientation += character.rotation * deltaTime;
                character.orientation = mapToRange(character.orientation);
            }
        } else {
            // When frozen, force the orientation to remain constant.
            //character.orientation = finalOrientation;
            character.velocity = sf::Vector2f(0,0);
            character.rotation = 0.f;
        }
        const float error = 5.f;
        if(distance < error) {
            character.velocity = sf::Vector2f(0,0);
            character.rotation = 0.f;
        }

        // Update sprite position and rotation.
        boidSprite.setPosition(character.position);
        boidSprite.setRotation(character.orientation * 180.f / PI);

        // Drop breadcrumbs.
        dropTimer += deltaTime;
        if (dropTimer >= dropInterval) {
            dropTimer = 0.f;
            breadcrumbs[crumbIndex].drop(character.position);
            crumbIndex = (crumbIndex + 1) % maxBreadcrumbs;
        }

        // Rendering.
        window.clear(sf::Color::White);
        for (auto& crumb : breadcrumbs)
            crumb.draw(&window);
        window.draw(boidSprite);
        window.display();
    }

    return 0;
}
