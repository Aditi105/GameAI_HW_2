#ifndef STEERING_BEHAVIORS_HPP
#define STEERING_BEHAVIORS_HPP

#include <SFML/System.hpp>

// A simple kinematic data structure
struct Kinematic {
    sf::Vector2f position;
    float orientation;   
    sf::Vector2f velocity;
    float rotation;      
};

struct SteeringOutput {
    sf::Vector2f linear;
    float angular;
};

// pure virtual base class for steering behaviors
class SteeringBehavior {
public:

    virtual SteeringOutput getSteering(const Kinematic& character, 
                                         const Kinematic& target, 
                                         float deltaTime) = 0;
    virtual ~SteeringBehavior() {}
};

// Matches the target's position
class PositionMatching : public SteeringBehavior {
public:
    virtual SteeringOutput getSteering(const Kinematic& character, 
                                       const Kinematic& target, 
                                       float deltaTime) override;
};

// Matches the target's orientation
class OrientationMatching : public SteeringBehavior {
public:
    virtual SteeringOutput getSteering(const Kinematic& character, 
                                       const Kinematic& target, 
                                       float deltaTime) override;
};

// Matches the target's velocity
class VelocityMatching : public SteeringBehavior {
public:
    virtual SteeringOutput getSteering(const Kinematic& character, 
                                       const Kinematic& target, 
                                       float deltaTime) override;
};

// Matches the target's rotation
class RotationMatching : public SteeringBehavior {
public:
    virtual SteeringOutput getSteering(const Kinematic& character, 
                                       const Kinematic& target, 
                                       float deltaTime) override;
};

#endif 
