#include <SFML/Graphics.hpp>
#include "Steering.hpp"
#include <cmath>
#include <deque>

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Velocity Matching Demo");

    // Load the texture from file.
    sf::Texture boidTexture;
    if (!boidTexture.loadFromFile("./src/boid-sm.png")) {
        return -1;
    }

    // Create a sprite using the texture.
    sf::Sprite boidSprite;
    boidSprite.setTexture(boidTexture);
    // Set the origin to the center of the sprite for proper positioning and rotation.
    sf::FloatRect spriteBounds = boidSprite.getLocalBounds();
    boidSprite.setOrigin(spriteBounds.width / 2.f, spriteBounds.height / 2.f);

    // Initialize the character's kinematic state.
    Kinematic character;
    character.position = sf::Vector2f(400.f, 300.f);
    character.velocity = sf::Vector2f(0.f, 0.f);
    character.orientation = 0.f;
    character.rotation = 0.f;

    // Create a VelocityMatching behavior with a specified maximum acceleration.
    float maxAcceleration = 300.f;
    VelocityMatching velocityMatching(maxAcceleration);

    sf::Clock clock;
    
    // Tuning parameters:
    float desiredSpeed = 150.f;       // Maximum desired speed (pixels/second)
    float stoppingDistance = 10.f;    // Distance threshold to snap to stop
    float velocityEpsilon = 1.f;      // Speed threshold to consider the character "stopped"
    const float PI = 3.14159265f;

    // The target position will be updated only when the mouse is clicked.
    sf::Vector2f targetPos = character.position;
    
    // Container to hold breadcrumb positions (maximum of 10).
    std::deque<sf::Vector2f> breadcrumbs;
    const std::size_t maxBreadcrumbs = 10;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            // Update the target position when the left mouse button is pressed.
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    targetPos = sf::Vector2f(static_cast<float>(event.mouseButton.x), 
                                             static_cast<float>(event.mouseButton.y));
                }
            }
        }

        float deltaTime = clock.restart().asSeconds();

        // Compute the vector from the character to the target position.
        sf::Vector2f toTarget = targetPos - character.position;
        float distance = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);

        // Compute desired velocity based on the clicked target.
        sf::Vector2f desiredVelocity(0.f, 0.f);
        if (distance > stoppingDistance) {
            desiredVelocity = (toTarget / distance) * desiredSpeed;
        } else {
            desiredVelocity = sf::Vector2f(0.f, 0.f);
        }

        // Build a target kinematic for velocity matching.
        Kinematic target;
        target.velocity = desiredVelocity;
        target.position = character.position;
        target.orientation = 0.f;
        target.rotation = 0.f;

        // Compute the steering output.
        SteeringOutput steering = velocityMatching.getSteering(character, target, deltaTime);

        // Update the character's velocity and position.
        character.velocity += steering.linear * deltaTime;
        character.position += character.velocity * deltaTime;
        
        // If the character is close enough and nearly stopped, snap it to the target.
        float speed = std::sqrt(character.velocity.x * character.velocity.x + character.velocity.y * character.velocity.y);
        if (distance < stoppingDistance && speed < velocityEpsilon) {
            character.velocity = sf::Vector2f(0.f, 0.f);
            character.position = targetPos;
        }
        
        // Update the sprite's position.
        boidSprite.setPosition(character.position);

        // Update the sprite's rotation based on the target direction.
        if (distance > 1.f) {  // Only update if the target is not exactly at the sprite's position.
            float angle = std::atan2(toTarget.y, toTarget.x) * 180.f / PI;
            boidSprite.setRotation(angle);
        }

        // Add current position to breadcrumbs.
        breadcrumbs.push_back(character.position);
        if (breadcrumbs.size() > maxBreadcrumbs) {
            breadcrumbs.pop_front();
        }

        window.clear(sf::Color::White);
        
        // Draw breadcrumbs (small circles) behind the sprite.
        for (const auto& pos : breadcrumbs) {
            sf::CircleShape crumb(3.f);
            crumb.setFillColor(sf::Color(200, 200, 200)); // Light gray
            crumb.setOrigin(3.f, 3.f);
            crumb.setPosition(pos);
            window.draw(crumb);
        }

        window.draw(boidSprite);
        window.display();
    }

    return 0;
}
