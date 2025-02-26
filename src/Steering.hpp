#ifndef STEERING_HPP 
#define STEERING_HPP

#include <SFML/Graphics.hpp>
#include <cmath>
#include <cstdlib>
#include <vector>


const float PI = 3.14159265f;


inline float vectorLength(const sf::Vector2f& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

// normalized vector in same direction as v
inline sf::Vector2f normalize(const sf::Vector2f& v) {
    float len = vectorLength(v);
    if (len != 0)
        return sf::Vector2f(v.x / len, v.y / len);
    return v;
}

// Fixes the value of vector v to a maximum value.
inline sf::Vector2f clamp(const sf::Vector2f& v, float maxVal) {
    float len = vectorLength(v);
    if (len > maxVal && len > 0)
        return normalize(v) * maxVal;
    return v;
}

// Fixes a scalar value to a maximum absolute value.
inline float clamp(float value, float maxVal) {
    if (std::abs(value) > maxVal)
        return (value > 0) ? maxVal : -maxVal;
    return value;
}


inline float mapToRange(float angle) {
    while (angle > PI) angle -= 2 * PI;
    while (angle < -PI) angle += 2 * PI;
    return angle;
}


struct Kinematic {
    sf::Vector2f position;
    sf::Vector2f velocity;
    float orientation; // in radians
    float rotation;    // radians per second
};

struct SteeringOutput {
    sf::Vector2f linear; 
    float angular;       // radians per second^2
};



class SteeringBehavior {
public:
    virtual ~SteeringBehavior() {}
    virtual SteeringOutput getSteering(const Kinematic& character, const Kinematic& target, float deltaTime) = 0;
};


// Arrive
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

        // If within the target radius, no steering.
        if (distance < targetRadius) {
            steering.linear = sf::Vector2f(0.f, 0.f);
            steering.angular = 0.f;
            return steering;
        }

        // If it is out of the slow radius, move at maximum speed.
        float targetSpeed = (distance > slowRadius) ? maxSpeed : maxSpeed * distance / slowRadius;
        sf::Vector2f desiredVelocity = normalize(direction) * targetSpeed;

        // the required acceleration to reach target.
        steering.linear = (desiredVelocity - character.velocity) / timeToTarget;
        steering.linear = clamp(steering.linear, maxAcceleration);
        steering.angular = 0.f;
        return steering;
    }

private:
    float maxAcceleration;
    float maxSpeed;
    float targetRadius;   // considered "arrived" if less than this radius
    float slowRadius;     // start slowing down when within this distance
    float timeToTarget;
};


// Align
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

        // If within the satisfaction radius, no steering
        if (rotationSize < satisfactionRadius) {
            steering.angular = 0.f;
            steering.linear = sf::Vector2f(0.f, 0.f);
            return steering;
        }

        
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
    float satisfactionRadius;   // If the rotation difference is less than this, no steering
    float decelerationRadius;   // Start decelerating rotation if less than this radius
    float timeToTarget;
};


// Wander
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

    virtual SteeringOutput getSteering(const Kinematic& character, const Kinematic& , float /*deltaTime*/) override {
        // update wander with random binomial value.
        wanderOrientation += randomBinomial() * wanderRate;
        float targetOrientation = character.orientation + wanderOrientation;
        
        // Calculating center of wander circle.
        sf::Vector2f circleCenter = character.position + normalize(character.velocity) * wanderOffset;
        // Calculating displacement from center.
        sf::Vector2f displacement(std::cos(targetOrientation), std::sin(targetOrientation));
        displacement *= wanderRadius;
        
        // Calculting target position on the wander circle.
        sf::Vector2f wanderTarget = circleCenter + displacement;
        
        
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

    
    float randomBinomial() {
        return ((float)std::rand() / RAND_MAX) - ((float)std::rand() / RAND_MAX);
    }
};



// Velocity Matching Behavior

class VelocityMatchingBehavior : public SteeringBehavior {
public:
    VelocityMatchingBehavior(float maxAccel, float timeToTarget)
        : maxAcceleration(maxAccel), timeToTarget(timeToTarget)
    {}

    virtual SteeringOutput getSteering(const Kinematic& character, const Kinematic& target, float /*deltaTime*/) override {
        SteeringOutput steering;
        // Calculating acceleration needed to match target velocity
        steering.linear = (target.velocity - character.velocity) / timeToTarget;
        steering.linear = clamp(steering.linear, maxAcceleration);
        steering.angular = 0.f;
        return steering;
    }

private:
    float maxAcceleration;
    float timeToTarget;
};



// Rotation Matching Behavior

class RotationMatchingBehavior : public SteeringBehavior {
public:
    RotationMatchingBehavior(float maxAngAccel, float timeToTarget)
        : maxAngularAcceleration(maxAngAccel), timeToTarget(timeToTarget)
    {}

    virtual SteeringOutput getSteering(const Kinematic& character, const Kinematic& target, float /*deltaTime*/) override {
        SteeringOutput steering;
        float rotationDiff = target.rotation - character.rotation;
        steering.angular = rotationDiff / timeToTarget;
        steering.angular = clamp(steering.angular, maxAngularAcceleration);
        steering.linear = sf::Vector2f(0.f, 0.f);
        return steering;
    }

private:
    float maxAngularAcceleration;
    float timeToTarget;
};

// Flocking Behavior

class FlockingBehavior : public SteeringBehavior {
public:
    FlockingBehavior(const std::vector<Kinematic>* flock,
                     float neighborRadius, float separationRadius,
                     float separationWeight, float alignmentWeight, float cohesionWeight,
                     float maxAcceleration,
                     //wander params
                     float wanderMaxAccel, float wanderMaxSpeed, float wanderOffset,
                     float wanderRadius, float wanderRate, float wanderTimeToTarget)
        : flock(flock), neighborRadius(neighborRadius), separationRadius(separationRadius),
          separationWeight(separationWeight), alignmentWeight(alignmentWeight), cohesionWeight(cohesionWeight),
          maxAcceleration(maxAcceleration),
          wander(wanderMaxAccel, wanderMaxSpeed, wanderOffset, wanderRadius, wanderRate, wanderTimeToTarget)
    {}

    virtual SteeringOutput getSteering(const Kinematic& character, const Kinematic& /*unused*/, float deltaTime) override {
        sf::Vector2f separation(0.f, 0.f);
        sf::Vector2f alignment(0.f, 0.f);
        sf::Vector2f cohesion(0.f, 0.f);
        int count = 0;
        for (const auto& other : *flock) {
            if (&other == &character)
                continue;
            sf::Vector2f toOther = other.position - character.position;
            float distance = vectorLength(toOther);
            if (distance < neighborRadius && distance > 0.f) {
                alignment += other.velocity;
                cohesion += other.position;
                count++;
                if (distance < separationRadius) {
                    separation += (character.position - other.position) / distance;
                }
            }
        }
        
        SteeringOutput steering;
        if (count == 0) {
            // If no neighbors, use wander
            return wander.getSteering(character, character, deltaTime);
        }
        
        alignment = alignment / static_cast<float>(count);
        cohesion = (cohesion / static_cast<float>(count)) - character.position;
        

        sf::Vector2f flockingForce = separation * separationWeight +
                                     alignment * alignmentWeight +
                                     cohesion * cohesionWeight;
        flockingForce = clamp(flockingForce, maxAcceleration);
        
        steering.linear = flockingForce;
        steering.angular = 0.f;
        return steering;
    }

private:
    const std::vector<Kinematic>* flock;
    float neighborRadius;
    float separationRadius;
    float separationWeight;
    float alignmentWeight;
    float cohesionWeight;
    float maxAcceleration;
    WanderBehavior wander;
};

#endif 
