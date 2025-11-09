#pragma once

#include <SFML/Graphics.hpp>
#include <functional>

//$ ----- Game Components ----- //

// Tag to identify the player entity
struct PlayerTag {};

struct Velocity { sf::Vector2f value{0.0f, 0.0f}; };

struct MovementSpeed { float value; };

// replace this with a sprite component later
struct PlayerCircle 
{
    PlayerCircle(float radius, const sf::Color& color, sf::Vector2f position)
        : shape()
    {
        shape.setRadius(radius);
        shape.setFillColor(color);
        shape.setOrigin({ radius, radius });
        shape.setPosition(position);
    }
    ~PlayerCircle() = default;

    sf::CircleShape shape; 
};


//$ ----- UI Components -----

// Tag to identify menu UI entities for easy cleanup
struct MenuUITag {};

struct UIText { sf::Text text; };

struct UIShape { sf::RectangleShape shape; };

// Defines the clickable/hoverable area
struct Bounds { sf::FloatRect rect; };

struct Clickable { std::function<void()> action; };

struct Hovered {};