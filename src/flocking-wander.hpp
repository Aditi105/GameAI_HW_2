#ifndef FLOCKING_WANDER_HPP
#define FLOCKING_WANDER_HPP

#include <SFML/Graphics.hpp>
#include <cmath>
#include <cstdlib>
#include <vector>


const float PI = 3.14159265f;

inline float vectorLength(const sf::Vector2f& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

inline sf::Vector2f normalize(const sf::Vector2f& v) {
    float len = vectorLength(v);
    if (len != 0)
        return sf::Vector2f(v.x / len, v.y / len);
    return v;
}

inline sf::Vector2f clamp(const sf::Vector2f& v, float maxVal) {
    float len = vectorLength(v);
    if (len > maxVal && len > 0)
        return normalize(v) * maxVal;
    return v;
}

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
    float orientation; // radians
    float rotation;    // radians per second
};

struct SteeringOutput {
    sf::Vector2f linear;
    float angular;       // (radians per second^2)
};



class SteeringBehavior {
public:
    virtual ~SteeringBehavior() {}
    virtual SteeringOutput getSteering(const Kinematic& character,
                                       const Kinematic& target,
                                       float deltaTime) = 0;
};



// Wanders by choosing a random target on a circle.
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

    virtual SteeringOutput getSteering(const Kinematic& character,
                                       const Kinematic&,
                                       float /*deltaTime*/) override {
        // taking a random binomial number to update wander
        wanderOrientation += randomBinomial() * wanderRate;
        float targetOrientation = character.orientation + wanderOrientation;
        
        // calculating center of the wander circle.
        sf::Vector2f circleCenter = character.position + normalize(character.velocity) * wanderOffset;
        // calculating point on the circle.
        sf::Vector2f displacement(std::cos(targetOrientation), std::sin(targetOrientation));
        displacement *= wanderRadius;
        
        // fixing the wander target.
        sf::Vector2f wanderTarget = circleCenter + displacement;
        
        // Arriving towards wander target
        sf::Vector2f direction = wanderTarget - character.position;
        float distance = vectorLength(direction);
        const float targetRadius = 5.f; 
        const float slowRadius = wanderRadius;
        if (distance < targetRadius) {
            return SteeringOutput{ sf::Vector2f(0.f, 0.f), 0.f };
        }
        float targetSpeed = (distance > slowRadius) ? maxSpeed : maxSpeed * distance / slowRadius;
        sf::Vector2f desiredVelocity = normalize(direction) * targetSpeed;
        sf::Vector2f acceleration = (desiredVelocity - character.velocity) / timeToTarget;
        acceleration = clamp(acceleration, maxAcceleration);
        return SteeringOutput{ acceleration, 0.f };
    }

private:
    float maxAcceleration;
    float maxSpeed;
    float wanderOffset;
    float wanderRadius;
    float wanderRate;
    float timeToTarget;
    float wanderOrientation;

    // random binomial value between -1 and 1
    float randomBinomial() {
        return ((float)std::rand() / RAND_MAX) - ((float)std::rand() / RAND_MAX);
    }
};


// If no neighbors are found, it goes back to the wander behavior.
class FlockingBehavior : public SteeringBehavior {
public:
    FlockingBehavior(const std::vector<Kinematic>* flock,
                     float neighborRadius, float separationRadius,
                     float separationWeight, float alignmentWeight, float cohesionWeight,
                     float maxAcceleration,
                     // Parameters for wander behavior:
                     float wanderMaxAccel, float wanderMaxSpeed, float wanderOffset,
                     float wanderRadius, float wanderRate, float wanderTimeToTarget)
        : flock(flock), neighborRadius(neighborRadius), separationRadius(separationRadius),
          separationWeight(separationWeight), alignmentWeight(alignmentWeight), cohesionWeight(cohesionWeight),
          maxAcceleration(maxAcceleration),
          wander(wanderMaxAccel, wanderMaxSpeed, wanderOffset, wanderRadius, wanderRate, wanderTimeToTarget)
    {}

    virtual SteeringOutput getSteering(const Kinematic& character,
                                       const Kinematic& ,
                                       float deltaTime) override {
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
        if (count == 0) {
            // if no neighbours, wander
            return wander.getSteering(character, character, deltaTime);
        }
        
        alignment /= static_cast<float>(count);
        cohesion = (cohesion / static_cast<float>(count)) - character.position;
        
        
        sf::Vector2f force = separation * separationWeight +
                             alignment * alignmentWeight +
                             cohesion * cohesionWeight;
        force = clamp(force, maxAcceleration);
        return SteeringOutput{ force, 0.f };
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
