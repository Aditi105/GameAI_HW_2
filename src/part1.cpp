#include "VelocityMatching.hpp"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>


// PositionMatching

SteeringOutput PositionMatching::getSteering(const Kinematic& character, const Kinematic& target, float deltaTime) {
    SteeringOutput output;
    // Compute desired velocity to reach target position in deltaTime
    sf::Vector2f desiredVelocity = (target.position - character.position) / deltaTime;
    // The linear acceleration needed is the difference between desired and current velocity
    output.linear = desiredVelocity - character.velocity;
    output.angular = 0.f;
    return output;
}


// OrientationMatching

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

//  VelocityMatching

SteeringOutput VelocityMatching::getSteering(const Kinematic& character, const Kinematic& target, float deltaTime) {
    SteeringOutput output;
    
    const float timeToTarget = 1.0f;  
    output.linear = (target.velocity - character.velocity) / timeToTarget;
    output.angular = 0.f;
    return output;
}


// RotationMatching

SteeringOutput RotationMatching::getSteering(const Kinematic& character, const Kinematic& target, float deltaTime) {
    SteeringOutput output;

    output.linear = sf::Vector2f(0.f, 0.f);
    output.angular = (target.rotation - character.rotation) / deltaTime;
    return output;
}

// Velocity and Orientation Matching 

int main() {
    // Create the SFML window
    sf::RenderWindow window(sf::VideoMode(640, 480), "Part 1");
    window.setFramerateLimit(60);

    
    sf::Texture boidTexture;
    if (!boidTexture.loadFromFile("src/boid-sm.png")) {
        std::cerr << "Error loading texture 'src/boid-sm.png'" << std::endl;
        return -1;
    }

    
    sf::Sprite boidSprite;
    boidSprite.setTexture(boidTexture);
    sf::FloatRect spriteBounds = boidSprite.getLocalBounds();
    boidSprite.setOrigin(spriteBounds.width / 2, spriteBounds.height / 2);
    boidSprite.setScale(4.f, 4.f);

    // Initializing character's kinematic state
    Kinematic character;
    character.position = sf::Vector2f(400.f, 300.f);
    character.velocity = sf::Vector2f(0.f, 0.f);
    character.orientation = 0.f;
    character.rotation = 0.f;

    VelocityMatching velocityMatching;

    // to computing mouse pointer velocity
    sf::Clock clock;
    sf::Vector2f previousMousePos = sf::Vector2f(sf::Mouse::getPosition(window));

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        float deltaTime = clock.restart().asSeconds();
        if (deltaTime == 0) continue; 

        // Sample the current mouse position
        sf::Vector2f currentMousePos = sf::Vector2f(sf::Mouse::getPosition(window));
        // Calculating mouse velocity
        sf::Vector2f mouseVelocity = (currentMousePos - previousMousePos) / deltaTime;
        previousMousePos = currentMousePos;

        // Creating a target kinematic based on mouse data.
        Kinematic target;
        target.position = currentMousePos;  
        target.velocity = mouseVelocity;
        target.orientation = 0.f;
        target.rotation = 0.f;

        // steering output from velocity matching
        SteeringOutput steering = velocityMatching.getSteering(character, target, deltaTime);

        // Updating the character's state
        character.velocity += steering.linear * deltaTime;
        character.position += character.velocity * deltaTime;

        // Updating orientation 
        if (std::abs(character.velocity.x) > 0.01f || std::abs(character.velocity.y) > 0.01f) {
            character.orientation = std::atan2(character.velocity.y, character.velocity.x);
        }

        // updating converting radians to degrees
        boidSprite.setPosition(character.position);
        boidSprite.setRotation(character.orientation * 180.f / 3.14159f);

        window.clear(sf::Color::White);
        window.draw(boidSprite);
        window.display();
    }

    return 0;
}
