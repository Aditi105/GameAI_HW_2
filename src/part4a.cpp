#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <ctime>
#include <vector>
#include "flocking-wander.hpp"

class crumb : public sf::CircleShape {
public:
    crumb(int id) {
        this->id = id;
        this->setRadius(1.5f);
        this->setFillColor(sf::Color(0, 0, 255, 255));
        this->setPosition(-100.f, -100.f);
    }

    void draw(sf::RenderWindow* window) {
        window->draw(*this);
    }

    void drop(float x, float y) {
        this->setPosition(x, y);
    }
    void drop(sf::Vector2f position) {
        this->setPosition(position);
    }

private:
    int id;
};

struct BoidBreadcrumbs {
    std::vector<crumb> crumbs;
    float drop_timer;
    int crumb_idx;

    BoidBreadcrumbs() : drop_timer(0.1f), crumb_idx(0) {
        for (int i = 0; i < 10; ++i) {
            crumbs.push_back(crumb(i));
        }
    }
};

const int windowWidth = 800;
const int windowHeight = 600;
const int numBoids = 150;

const float neighborRadius    = 20.f;
const float separationRadius  = 20.f;
const float separationWeight  = 5.0f;
const float alignmentWeight   = 1.f;
const float cohesionWeight    = 1.0f;
const float maxAccel          = 250.f;

const float wanderMaxAccel    = 5.f;
const float wanderMaxSpeed    = 7.f;
const float wanderOffset      = 10.f;
const float wanderRadius      = 15.f;
const float wanderRate        = 1.0f;
const float wanderTimeToTarget= 0.1f;

const float initialSpeed      = 13.f;
const float maxSpeed          = 13.f;

int main()
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    sf::Texture boidTexture;
    if (!boidTexture.loadFromFile("src/boid-sm.png"))
    {
        return -1;
    }
    sf::Vector2u texSize = boidTexture.getSize();
    sf::Vector2f textureOrigin(texSize.x / 2.f, texSize.y / 2.f);

    std::vector<Kinematic> flock;
    std::vector<FlockingBehavior> behaviors;
    std::vector<sf::Sprite> sprites;
    std::vector<BoidBreadcrumbs> boidBreadcrumbs;

    for (int i = 0; i < numBoids; ++i)
    {
        Kinematic k;
        k.position = sf::Vector2f(static_cast<float>(std::rand() % windowWidth),
                                  static_cast<float>(std::rand() % windowHeight));
        float angle = (std::rand() % 360) * (PI / 180.f);
        k.velocity = sf::Vector2f(std::cos(angle), std::sin(angle)) * initialSpeed;
        k.orientation = angle;
        k.rotation = 0.f;
        flock.push_back(k);

        behaviors.push_back(FlockingBehavior(&flock,
                                               neighborRadius, separationRadius,
                                               separationWeight, alignmentWeight, cohesionWeight,
                                               maxAccel,
                                               wanderMaxAccel, wanderMaxSpeed, wanderOffset,
                                               wanderRadius, wanderRate, wanderTimeToTarget));

        sf::Sprite sprite;
        sprite.setTexture(boidTexture);
        sprite.setOrigin(textureOrigin);
        sprite.setScale(1.f, 1.f);
        sprites.push_back(sprite);

        boidBreadcrumbs.push_back(BoidBreadcrumbs());
    }

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Part 4");
    window.setFramerateLimit(60);
    sf::Clock clock;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        sf::Time dt = clock.restart();
        float deltaTime = dt.asSeconds();

        for (int i = 0; i < numBoids; ++i)
        {
            SteeringOutput steering = behaviors[i].getSteering(flock[i], flock[i], deltaTime);
            flock[i].velocity += steering.linear * deltaTime;
            flock[i].velocity = clamp(flock[i].velocity, maxSpeed);
            flock[i].position += flock[i].velocity * deltaTime;

            if (flock[i].position.x < 0) flock[i].position.x += windowWidth;
            if (flock[i].position.y < 0) flock[i].position.y += windowHeight;
            if (flock[i].position.x > windowWidth) flock[i].position.x -= windowWidth;
            if (flock[i].position.y > windowHeight) flock[i].position.y -= windowHeight;

            if (vectorLength(flock[i].velocity) > 0)
                flock[i].orientation = std::atan2(flock[i].velocity.y, flock[i].velocity.x);
        }

        for (int i = 0; i < numBoids; ++i)
        {
            boidBreadcrumbs[i].drop_timer -= deltaTime;
            if (boidBreadcrumbs[i].drop_timer <= 0.f)
            {
                boidBreadcrumbs[i].drop_timer = 0.3f;
                boidBreadcrumbs[i].crumbs[boidBreadcrumbs[i].crumb_idx].drop(flock[i].position);
                boidBreadcrumbs[i].crumb_idx = (boidBreadcrumbs[i].crumb_idx + 1) % 10;
            }
        }

        window.clear(sf::Color::White);

        for (int i = 0; i < numBoids; ++i)
        {
            for (auto& c : boidBreadcrumbs[i].crumbs)
            {
                c.draw(&window);
            }
        }

        for (int i = 0; i < numBoids; ++i)
        {
            sprites[i].setPosition(flock[i].position);
            sprites[i].setRotation(flock[i].orientation * 180.f / PI);
            window.draw(sprites[i]);
        }

        window.display();
    }

    return 0;
}
