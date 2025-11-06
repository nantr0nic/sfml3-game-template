#include "Player.hpp"

Player::Player(float posX, float posY)
{
    m_PlayerShape.setRadius(m_Radius);
    m_PlayerShape.setFillColor(m_Color);
    m_PlayerShape.setOrigin({ m_Radius, m_Radius }); // Center the origin
    m_PlayerShape.setPosition({ posX, posY });
}

void Player::handleInput()
{
    m_Velocity = { 0.0f, 0.0f };

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::W))
    {
        m_Velocity.y -= m_Speed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::S))
    {
        m_Velocity.y += m_Speed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::A))
    {
        m_Velocity.x -= m_Speed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::D))
    {
        m_Velocity.x += m_Speed;
    }
}

void Player::update(sf::Time deltaTime)
{
    m_PlayerShape.move(m_Velocity * deltaTime.asSeconds());
}

void Player::render(sf::RenderWindow& window)
{
    window.draw(m_PlayerShape);
}

sf::Vector2f Player::getPosition() const
{
    return m_PlayerShape.getPosition();
}