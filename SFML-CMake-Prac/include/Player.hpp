/*
I want to learn how to make Player as a component in an ECS system;
but until then, we'll use this generic Player class.
*/
#pragma once

#include <SFML/Graphics.hpp>

class Player 
{
public:
    Player(float posX, float posY);
    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;
    ~Player() = default;

    void handleInput();
    void update(sf::Time deltaTime);
    void render(sf::RenderWindow& window);

    sf::Vector2f getPosition() const;

private:
    sf::CircleShape m_PlayerShape;
    float m_Radius{ 50.0f };
    sf::Color m_Color{ sf::Color::Green };

    sf::Vector2f m_Velocity;
    const float m_Speed{ 200.0f }; // pixels per second

};