#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include "Steering.hpp"


const sf::Vector2f TOP_RIGHT(550, 0);
const sf::Vector2f BOT_RIGHT(550, 550);
const sf::Vector2f BOT_LEFT(0, 550);
const sf::Vector2f TOP_LEFT(0, 0);

class crumb : public sf::CircleShape {
public:
    crumb(int id) : id(id) {
        this->setRadius(5.f);
        this->setFillColor(sf::Color(0, 0, 255, 255));
        this->setPosition(-100.f, -100.f);
    }

    void draw(sf::RenderWindow* window) {
        window->draw(*this);
    }

    // set position of breadcrumb
    void drop(float x, float y) {
        this->setPosition(x, y);
    }
    void drop(const sf::Vector2f& position) {
        this->setPosition(position);
    }

private:
    int id;
};


class Boid {
public:
    Boid(sf::RenderWindow* win, std::vector<crumb>* crumbs, const sf::Texture& texture)
        : window(win), breadcrumbs(crumbs)
    {
        kinematic.position = sf::Vector2f(300.f, 300.f);
        kinematic.velocity = sf::Vector2f(50.f, 0.f); // initial velocity
        kinematic.orientation = 0.f;
        kinematic.rotation = 0.f;
        maxSpeed = 100.f;
        maxAcceleration = 50.f;
        wanderBehavior = new WanderBehavior(maxAcceleration, maxSpeed,
                                            150.f,  // wander offset
                                            30.f,  // wander circle radius
                                            0.5f,  // wander rate (radians per update)
                                            0.1f); // time to target for Arrive part

        boidSprite.setTexture(texture);
        sf::FloatRect bounds = boidSprite.getLocalBounds();
        boidSprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
        boidSprite.setScale(4.f, 4.f);
        boidSprite.setPosition(kinematic.position);

        dropTimer = 0.f;
        dropInterval = 0.2f; // drop a crumb every 0.2 seconds
    }

    ~Boid() {
        delete wanderBehavior;
    }

    // update boid kinematics, drop breadcrumbs, and handle boundaries
    void update(float deltaTime) {
        SteeringOutput steering = wanderBehavior->getSteering(kinematic, kinematic, deltaTime);

        kinematic.velocity += steering.linear * deltaTime;
        if (vectorLength(kinematic.velocity) > maxSpeed)
            kinematic.velocity = normalize(kinematic.velocity) * maxSpeed;
        kinematic.position += kinematic.velocity * deltaTime;

        if (vectorLength(kinematic.velocity) > 0.001f)
            kinematic.orientation = std::atan2(kinematic.velocity.y, kinematic.velocity.x);

        // Update the sprite position and rotation.
        boidSprite.setPosition(kinematic.position);
        boidSprite.setRotation(kinematic.orientation * 180 / PI);

    

        // Boundary handling
        sf::Vector2u winSize = window->getSize();
        if (kinematic.position.x < 0.f)
            kinematic.position.x = static_cast<float>(winSize.x);
        else if (kinematic.position.x > winSize.x)
            kinematic.position.x = 0.f;

        if (kinematic.position.y < 0.f)
            kinematic.position.y = static_cast<float>(winSize.y);
        else if (kinematic.position.y > winSize.y)
            kinematic.position.y = 0.f;


        // Update breadcrumb timer and drop crumbs.
        dropTimer += deltaTime;
        if (dropTimer >= dropInterval) {
            dropTimer = 0.f;
            breadcrumbs->at(crumbIndex).drop(kinematic.position);
            crumbIndex = (crumbIndex + 1) % breadcrumbs->size();
        }
    }

    void draw() {
        window->draw(boidSprite);
    }

private:
    sf::RenderWindow* window;
    Kinematic kinematic;
    float maxSpeed;
    float maxAcceleration;
    WanderBehavior* wanderBehavior;

    sf::Sprite boidSprite;

    std::vector<crumb>* breadcrumbs;
    float dropTimer;
    float dropInterval;
    size_t crumbIndex = 0;
};


int main()
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    sf::RenderWindow window(sf::VideoMode(640, 480), "Part 3");
    window.setFramerateLimit(60);

    sf::Texture boidTexture;
    if (!boidTexture.loadFromFile("./src/boid-sm.png")) {
        return -1;
    }

    std::vector<crumb> breadcrumbs;
    for (int i = 0; i < 20; i++) {
        breadcrumbs.push_back(crumb(i));
    }

    Boid boid(&window, &breadcrumbs, boidTexture);

    sf::Clock clock;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        
        float deltaTime = clock.restart().asSeconds();
        boid.update(deltaTime);

        window.clear(sf::Color::White);
        for (auto& c : breadcrumbs)
            c.draw(&window);
        boid.draw();
        window.display();
    }
    return 0;
}
