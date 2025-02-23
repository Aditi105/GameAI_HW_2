#ifndef STEERING_HPP 
#define STEERING_HPP

#include <SFML/Graphics.hpp>
#include <cmath>
#include <cstdlib>

// -----------------------------------------------------------------
// Utility Functions
// -----------------------------------------------------------------

const float PI = 3.14159265f;

// Returns the length (magnitude) of a 2D vector.
inline float vectorLength(const sf::Vector2f& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

// Returns a normalized (unit) vector in the same direction as v.
inline sf::Vector2f normalize(const sf::Vector2f& v) {
    float len = vectorLength(v);
    if (len != 0)
        return sf::Vector2f(v.x / len, v.y / len);
    return v;
}

// Clamps the magnitude of vector v to a maximum value.
inline sf::Vector2f clamp(const sf::Vector2f& v, float maxVal) {
    float len = vectorLength(v);
    if (len > maxVal && len > 0)
        return normalize(v) * maxVal;
    return v;
}

// Clamps a scalar value to a maximum absolute value.
inline float clamp(float value, float maxVal) {
    if (std::abs(value) > maxVal)
        return (value > 0) ? maxVal : -maxVal;
    return value;
}

// Maps an angle (in radians) into the range [-PI, PI].
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
// This behavior makes the character accelerate toward the target and decelerate as it nears the target.
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
        float distance = vectorLength(direction);

        // If within the target radius, no steering is needed.
        if (distance < targetRadius) {
            steering.linear = sf::Vector2f(0.f, 0.f);
            steering.angular = 0.f;
            return steering;
        }

        // If outside the slow radius, move at maximum speed.
        float targetSpeed = (distance > slowRadius) ? maxSpeed : maxSpeed * distance / slowRadius;
        sf::Vector2f desiredVelocity = normalize(direction) * targetSpeed;

        // Calculate required acceleration.
        steering.linear = (desiredVelocity - character.velocity) / timeToTarget;
        steering.linear = clamp(steering.linear, maxAcceleration);
        steering.angular = 0.f;
        return steering;
    }

private:
    float maxAcceleration;
    float maxSpeed;
    float targetRadius;   // Within this distance, the character is considered "arrived."
    float slowRadius;     // Begin slowing down when within this distance.
    float timeToTarget;
};

// -----------------------------------------------------------------
// Align Behavior
// -----------------------------------------------------------------
// This behavior smoothly rotates the character so that its orientation matches the target's orientation.
class AlignBehavior : public SteeringBehavior {
public:
    AlignBehavior(float maxAngAccel, float maxRot, float satisfactionRadius,
                  float decelerationRadius, float timeToTarget)
        : maxAngularAcceleration(maxAngAccel), maxRotation(maxRot),
          satisfactionRadius(satisfactionRadius), decelerationRadius(decelerationRadius),
          timeToTarget(timeToTarget)
    {}

    virtual SteeringOutput getSteering(const Kinematic& character, const Kinematic& target, float /*deltaTime*/) override {
        SteeringOutput steering;
        float rotation = target.orientation - character.orientation;
        rotation = mapToRange(rotation);
        float rotationSize = std::abs(rotation);

        // If within the satisfaction radius, no steering is needed.
        if (rotationSize < satisfactionRadius) {
            steering.angular = 0.f;
            steering.linear = sf::Vector2f(0.f, 0.f);
            return steering;
        }

        // Calculate desired rotation.
        float desiredRotation = (rotationSize > decelerationRadius) ? maxRotation : maxRotation * rotationSize / decelerationRadius;
        desiredRotation *= (rotation / rotationSize);
        steering.angular = (desiredRotation - character.rotation) / timeToTarget;
        steering.angular = clamp(steering.angular, maxAngularAcceleration);
        steering.linear = sf::Vector2f(0.f, 0.f);
        return steering;
    }

private:
    float maxAngularAcceleration;
    float maxRotation;
    float satisfactionRadius;   // If the rotation difference is within this, no steering is applied.
    float decelerationRadius;   // Begin decelerating rotation within this range.
    float timeToTarget;
};

// -----------------------------------------------------------------
// Wander Behavior
// -----------------------------------------------------------------
// This behavior causes the character to move forward while randomly changing direction.
// It works by selecting a target point on a circle (offset in front of the character)
// and then using an Arrive behavior to steer toward that target.
class WanderBehavior : public SteeringBehavior {
public:
    WanderBehavior(float maxAccel, float maxSpeed,
                   float wanderOffset, float wanderRadius,
                   float wanderRate, float timeToTarget)
        : maxAcceleration(maxAccel), maxSpeed(maxSpeed),
          wanderOffset(wanderOffset), wanderRadius(wanderRadius),
          wanderRate(wanderRate), timeToTarget(timeToTarget),
          wanderOrientation(0.f)
    {}

    virtual SteeringOutput getSteering(const Kinematic& character, const Kinematic& /*unused*/, float /*deltaTime*/) override {
        // Update the wander orientation with a random binomial value.
        wanderOrientation += randomBinomial() * wanderRate;
        float targetOrientation = character.orientation + wanderOrientation;
        
        // Calculate the center of the wander circle.
        sf::Vector2f circleCenter = character.position + normalize(character.velocity) * wanderOffset;
        // Calculate the displacement from the center.
        sf::Vector2f displacement(std::cos(targetOrientation), std::sin(targetOrientation));
        displacement *= wanderRadius;
        
        // Compute the target position on the wander circle.
        sf::Vector2f wanderTarget = circleCenter + displacement;
        
        // Use Arrive behavior to steer toward the wander target.
        Kinematic dummyTarget;
        dummyTarget.position = wanderTarget;
        dummyTarget.velocity = sf::Vector2f(0.f, 0.f);
        dummyTarget.orientation = 0.f;
        dummyTarget.rotation = 0.f;
        ArriveBehavior arrive(maxAcceleration, maxSpeed, 5.f, wanderRadius, timeToTarget);
        return arrive.getSteering(character, dummyTarget, 0.f);
    }
    
private:
    float maxAcceleration;
    float maxSpeed;
    float wanderOffset;
    float wanderRadius;
    float wanderRate;
    float timeToTarget;
    float wanderOrientation;

    // Returns a random value between -1 and 1 (roughly binomially distributed).
    float randomBinomial() {
        return ((float)std::rand() / RAND_MAX) - ((float)std::rand() / RAND_MAX);
    }
};

#endif // STEERING_HPP
