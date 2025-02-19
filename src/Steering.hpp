#ifndef STEERING_HPP
#define STEERING_HPP

#include <SFML/Graphics.hpp>
#include <cmath>

// -----------------------------------------------------------------
// Utility functions
// -----------------------------------------------------------------

const float PI = 3.14159265f;

// Returns the length of a 2D vector.
inline float length(const sf::Vector2f& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

// Returns a normalized copy of a vector (if length is nonzero).
inline sf::Vector2f normalize(const sf::Vector2f& v) {
    float len = length(v);
    if (len != 0)
        return sf::Vector2f(v.x / len, v.y / len);
    return v;
}

// Maps an angle (in radians) to the range [-PI, PI].
inline float mapToRange(float angle) {
    while (angle > PI) angle -= 2 * PI;
    while (angle < -PI) angle += 2 * PI;
    return angle;
}

// -----------------------------------------------------------------
// Data Structures for Steering
// -----------------------------------------------------------------

struct Kinematic {
    sf::Vector2f position;
    sf::Vector2f velocity;
    float orientation; // in radians
    float rotation;    // angular velocity (radians per second)
};

struct SteeringOutput {
    sf::Vector2f linear; // linear acceleration
    float angular;       // angular acceleration (radians per second^2)
};

// -----------------------------------------------------------------
// Abstract Steering Behavior Class
// -----------------------------------------------------------------

class SteeringBehavior {
public:
    virtual ~SteeringBehavior() {}
    virtual SteeringOutput getSteering(const Kinematic& character, const Kinematic& target, float deltaTime) = 0;
};

// -----------------------------------------------------------------
// Arrive Behavior
// -----------------------------------------------------------------
// This behavior causes the boid to slow down as it nears the target.
class ArriveBehavior : public SteeringBehavior {
public:
    ArriveBehavior(float maxAccel, float maxSpeed, float targetRadius, float slowRadius, float timeToTarget)
        : maxAcceleration(maxAccel), maxSpeed(maxSpeed),
          targetRadius(targetRadius), slowRadius(slowRadius),
          timeToTarget(timeToTarget)
    {}

    virtual SteeringOutput getSteering(const Kinematic& character, const Kinematic& target, float /*deltaTime*/) override {
        SteeringOutput steering;
        sf::Vector2f direction = target.position - character.position;
        float distance = length(direction);

        // If we're within the target radius, no steering is needed.
        if (distance < targetRadius) {
            steering.linear = sf::Vector2f(0.f, 0.f);
            steering.angular = 0.f;
            return steering;
        }

        // Determine the target speed.
        float targetSpeed = (distance > slowRadius) ? maxSpeed : maxSpeed * distance / slowRadius;
        
        // Calculate the desired velocity.
        sf::Vector2f desiredVelocity = normalize(direction) * targetSpeed;
        
        // Calculate acceleration required to reach the desired velocity.
        steering.linear = (desiredVelocity - character.velocity) / timeToTarget;
        
        // Clamp the acceleration if necessary.
        if (length(steering.linear) > maxAcceleration) {
            steering.linear = normalize(steering.linear) * maxAcceleration;
        }
        
        steering.angular = 0.f;
        return steering;
    }

private:
    float maxAcceleration;
    float maxSpeed;
    float targetRadius;   // when within this distance, we consider arrived
    float slowRadius;     // start decelerating within this distance
    float timeToTarget;
};

// -----------------------------------------------------------------
// Align Behavior
// -----------------------------------------------------------------
// This behavior rotates the boid smoothly to match the target orientation.
class AlignBehavior : public SteeringBehavior {
public:
    AlignBehavior(float maxAngAccel, float maxRotation, float satisfactionRadius,
                  float decelerationRadius, float timeToTarget)
        : maxAngularAcceleration(maxAngAccel), maxRotation(maxRotation),
          satisfactionRadius(satisfactionRadius), decelerationRadius(decelerationRadius),
          timeToTarget(timeToTarget)
    {}

    virtual SteeringOutput getSteering(const Kinematic& character, const Kinematic& target, float /*deltaTime*/) override {
        SteeringOutput steering;
        float rotation = target.orientation - character.orientation;
        rotation = mapToRange(rotation);
        float rotationSize = std::abs(rotation);

        // If within satisfaction radius, no angular steering is needed.
        if (rotationSize < satisfactionRadius) {
            steering.angular = 0.f;
            steering.linear = sf::Vector2f(0.f, 0.f);
            return steering;
        }
        
        // Determine the desired rotation speed.
        float desiredRotation = (rotationSize > decelerationRadius) ? maxRotation
                               : maxRotation * rotationSize / decelerationRadius;
        // Desired rotation maintains the sign of the original rotation.
        desiredRotation *= (rotation / rotationSize);
        
        // Calculate the angular acceleration.
        steering.angular = (desiredRotation - character.rotation) / timeToTarget;
        
        // Clamp the angular acceleration.
        if (std::abs(steering.angular) > maxAngularAcceleration) {
            steering.angular = (steering.angular / std::abs(steering.angular)) * maxAngularAcceleration;
        }
        
        steering.linear = sf::Vector2f(0.f, 0.f);
        return steering;
    }

private:
    float maxAngularAcceleration;
    float maxRotation;
    float satisfactionRadius;   // in radians: if within, no rotation is needed.
    float decelerationRadius;   // begin decelerating rotation within this range.
    float timeToTarget;
};

#endif // STEERING_HPP
