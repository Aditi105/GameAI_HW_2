#ifndef STEERING_HPP
#define STEERING_HPP

#include <SFML/System.hpp>
#include <cmath>

//------------------------------
// Data Structures
//------------------------------

// Holds the kinematic state.
struct Kinematic {
    sf::Vector2f position;
    float orientation;
    sf::Vector2f velocity;
    float rotation;
};

// Holds the computed steering output.
struct SteeringOutput {
    sf::Vector2f linear; // Linear acceleration
    float angular;       // Angular acceleration
};

//------------------------------
// Steering Behaviors
//------------------------------

// Pure virtual base class for steering behaviors.
class SteeringBehavior {
public:
    virtual ~SteeringBehavior() {}
    // Computes steering output based on character & target kinematics.
    virtual SteeringOutput getSteering(const Kinematic& character, const Kinematic& target, float deltaTime) = 0;
};

// Velocity Matching: Computes a steering force so the character’s velocity matches a target velocity.
class VelocityMatching : public SteeringBehavior {
public:
    VelocityMatching(float maxAcceleration)
        : m_maxAcceleration(maxAcceleration) {}

    virtual SteeringOutput getSteering(const Kinematic& character, const Kinematic& target, float deltaTime) override {
        SteeringOutput steering;
        // Calculate the difference between target and current velocity.
        sf::Vector2f velocityDiff = target.velocity - character.velocity;
        steering.linear = velocityDiff / deltaTime;
        
        // Clamp acceleration.
        float mag = std::sqrt(steering.linear.x * steering.linear.x + steering.linear.y * steering.linear.y);
        if(mag > m_maxAcceleration) {
            steering.linear = (steering.linear / mag) * m_maxAcceleration;
        }
        
        steering.angular = 0.f;
        return steering;
    }
    
private:
    float m_maxAcceleration;
};

#endif // STEERING_HPP
