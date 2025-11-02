#include "Player.hpp"

Player::Player(float posX, float posY)
{
    playerShape.setRadius(mRadius);
    playerShape.setFillColor(mColor);
    playerShape.setOrigin({ mRadius, mRadius }); // Center the origin
    playerShape.setPosition({ posX, posY });
}

void Player::handleInput()
{
    mVelocity = { 0.0f, 0.0f };

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::W))
    {
        mVelocity.y -= mSpeed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::S))
    {
        mVelocity.y += mSpeed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::A))
    {
        mVelocity.x -= mSpeed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::D))
    {
        mVelocity.x += mSpeed;
    }
}

void Player::update(sf::Time deltaTime)
{
    playerShape.move(mVelocity * deltaTime.asSeconds());
}

void Player::render(sf::RenderWindow& window)
{
    window.draw(playerShape);
}

sf::Vector2f Player::getPosition() const
{
    return playerShape.getPosition();
}