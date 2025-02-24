#include "VelocityMatching.hpp"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>

// ---------------------------
// Implementation of PositionMatching
// ---------------------------
SteeringOutput PositionMatching::getSteering(const Kinematic& character, const Kinematic& target, float deltaTime) {
    SteeringOutput output;
    // Compute desired velocity to reach target position in deltaTime
    sf::Vector2f desiredVelocity = (target.position - character.position) / deltaTime;
    // The linear acceleration needed is the difference between desired and current velocity
    output.linear = desiredVelocity - character.velocity;
    output.angular = 0.f;
    return output;
}

// ---------------------------
// Implementation of OrientationMatching
// ---------------------------
SteeringOutput OrientationMatching::getSteering(const Kinematic& character, const Kinematic& target, float deltaTime) {
    SteeringOutput output;
    // Compute smallest angular difference
    float diff = target.orientation - character.orientation;
    while (diff > 3.14159f) diff -= 2.f * 3.14159f;
    while (diff < -3.14159f) diff += 2.f * 3.14159f;
    // Desired angular velocity to cover the difference in deltaTime
    float desiredAngularVelocity = diff / deltaTime;
    output.linear = sf::Vector2f(0.f, 0.f);
    output.angular = desiredAngularVelocity - character.rotation;
    return output;
}

// ---------------------------
// Modified Implementation of VelocityMatching
// ---------------------------
SteeringOutput VelocityMatching::getSteering(const Kinematic& character, const Kinematic& target, float deltaTime) {
    SteeringOutput output;
    // Use a constant time-to-target to lower acceleration and smooth movement.
    // Increasing timeToTarget makes the boid take longer to match the target velocity.
    const float timeToTarget = 1.0f;  // seconds
    output.linear = (target.velocity - character.velocity) / timeToTarget;
    output.angular = 0.f;
    return output;
}

// ---------------------------
// Implementation of RotationMatching
// ---------------------------
SteeringOutput RotationMatching::getSteering(const Kinematic& character, const Kinematic& target, float deltaTime) {
    SteeringOutput output;
    // Compute the angular acceleration needed to match target rotation
    output.linear = sf::Vector2f(0.f, 0.f);
    output.angular = (target.rotation - character.rotation) / deltaTime;
    return output;
}

// ---------------------------
// Main Demo: Velocity and Orientation Matching with SFML and Sprite
// ---------------------------
int main() {
    // Create the SFML window
    sf::RenderWindow window(sf::VideoMode(640, 480), "Part 1");
    window.setFramerateLimit(60);

    // Load the texture from the file "src/boid-sm.png"
    sf::Texture boidTexture;
    if (!boidTexture.loadFromFile("src/boid-sm.png")) {
        std::cerr << "Error loading texture 'src/boid-sm.png'" << std::endl;
        return -1;
    }

    // Create a sprite and set its texture
    sf::Sprite boidSprite;
    boidSprite.setTexture(boidTexture);
    // Set the origin to the center of the sprite for correct positioning/rotation
    sf::FloatRect spriteBounds = boidSprite.getLocalBounds();
    boidSprite.setOrigin(spriteBounds.width / 2, spriteBounds.height / 2);
    // Scale the boid up by a factor of 4
    boidSprite.setScale(4.f, 4.f);

    // Initialize the character's kinematic state
    Kinematic character;
    character.position = sf::Vector2f(400.f, 300.f);
    character.velocity = sf::Vector2f(0.f, 0.f);
    character.orientation = 0.f;
    character.rotation = 0.f;

    // Instantiate the velocity matching behavior
    VelocityMatching velocityMatching;

    // For computing mouse pointer velocity
    sf::Clock clock;
    sf::Vector2f previousMousePos = sf::Vector2f(sf::Mouse::getPosition(window));

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Determine elapsed time
        float deltaTime = clock.restart().asSeconds();
        if (deltaTime == 0) continue; // safeguard against division by zero

        // Sample the current mouse position
        sf::Vector2f currentMousePos = sf::Vector2f(sf::Mouse::getPosition(window));
        // Calculate mouse velocity: (current position - previous position) / deltaTime
        sf::Vector2f mouseVelocity = (currentMousePos - previousMousePos) / deltaTime;
        previousMousePos = currentMousePos;

        // Create a target kinematic based on mouse data.
        // For velocity matching, we are interested in the mouse pointer's velocity.
        Kinematic target;
        target.position = currentMousePos;  // (optional for other behaviors)
        target.velocity = mouseVelocity;
        target.orientation = 0.f;
        target.rotation = 0.f;

        // Get the steering output from velocity matching
        SteeringOutput steering = velocityMatching.getSteering(character, target, deltaTime);

        // Update the character's state using simple Euler integration
        character.velocity += steering.linear * deltaTime;
        character.position += character.velocity * deltaTime;

        // Update orientation based on velocity direction if velocity is significant
        if (std::abs(character.velocity.x) > 0.01f || std::abs(character.velocity.y) > 0.01f) {
            character.orientation = std::atan2(character.velocity.y, character.velocity.x);
        }

        // Update the sprite's position and rotation (convert radians to degrees)
        boidSprite.setPosition(character.position);
        boidSprite.setRotation(character.orientation * 180.f / 3.14159f);

        // Render with a white background
        window.clear(sf::Color::White);
        window.draw(boidSprite);
        window.display();
    }

    return 0;
}
