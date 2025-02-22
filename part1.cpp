//part1

#include <SFML/Graphics.hpp>
#include "Steering1.hpp"
#include <iostream>

int main() {
    sf::RenderWindow window(sf::VideoMode(800,600), "Velocity Matching Demo (Steering1)");

    // Create a visual representation for the character (a blue circle).
    sf::CircleShape characterShape(20.f);
    characterShape.setFillColor(sf::Color::Blue);
    characterShape.setOrigin(20.f, 20.f);

    // Initialize the character's kinematic state.
    Kinematic character;
    character.position = sf::Vector2f(400.f, 300.f);
    character.velocity = sf::Vector2f(0.f, 0.f);
    character.orientation = 0.f;
    character.rotation = 0.f;

    // Create a VelocityMatching behavior with a specified maximum acceleration.
    float maxAccel = 500.f;
    VelocityMatching velocityMatching(maxAccel);

    sf::Clock clock;
    // Initialize previous mouse position.
    sf::Vector2i previousMouse = sf::Mouse::getPosition(window);

    while(window.isOpen()) {
        sf::Event event;
        while(window.pollEvent(event)) {
            if(event.type == sf::Event::Closed)
                window.close();
        }

        float deltaTime = clock.restart().asSeconds();

        // Sample current mouse position.
        sf::Vector2i currentMouseInt = sf::Mouse::getPosition(window);
        sf::Vector2f currentMouse(static_cast<float>(currentMouseInt.x), static_cast<float>(currentMouseInt.y));
        sf::Vector2f prevMouse(static_cast<float>(previousMouse.x), static_cast<float>(previousMouse.y));

        // Compute the mouse pointer's velocity.
        sf::Vector2f pointerVelocity = (currentMouse - prevMouse) / deltaTime;
        previousMouse = currentMouseInt;

        // Build a target kinematic for velocity matching.
        Kinematic target;
        target.position = character.position; // not used in velocity matching
        target.velocity = pointerVelocity;
        target.orientation = 0.f; // not used
        target.rotation = 0.f;    // not used

        // Compute the steering output to match the mouse pointer's velocity.
        SteeringOutput steering = velocityMatching.getSteering(character, target, deltaTime);

        // Update the character's velocity and position.
        character.velocity += steering.linear * deltaTime;
        character.position += character.velocity * deltaTime;

        // Update the visual representation.
        characterShape.setPosition(character.position);

        window.clear(sf::Color::White);
        window.draw(characterShape);
        window.display();
    }

    return 0;
}
