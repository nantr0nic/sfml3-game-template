#pragma once

#include <SFML/Graphics.hpp>

#include "Utils.hpp"

#include <functional>
#include <map>

//$ ----- Game Components ----- //

struct PlayerTag {}; // Tag to identify the player entity

struct Velocity { sf::Vector2f value{0.0f, 0.0f}; };

struct MovementSpeed { float value; };

struct BoundaryHits
{
    bool north = false;
    bool south = false;
    bool west = false;
    bool east = false;
};

// ----- Sprite / Animation Components ----- //
struct SpriteComponent 
{
    SpriteComponent(const sf::Texture& texture)
        : sprite(texture)
    { }

    ~SpriteComponent() = default;

    sf::Sprite sprite; 
};

struct Animation
{
    Animation() = default;
    Animation(int row, int frames, sf::Time duration)
        : row(row)
        , frames(frames)
        , duration(duration)
    { }

    ~Animation() = default;


    int row;           // The row on the sprite sheet (0 for idle, 1 for walk, etc.)
    int frames;        // Total number of frames in this animation
    sf::Time duration; // Total duration this animation should take
    
    // Calculates time per frame. sf::Time / float is a valid operation.
    sf::Time getTimePerFrame() const { return duration / (float)frames; }
};

struct AnimatorComponent
{
    // Stores all available animations by name 
    std::map<std::string, Animation> animations;

    std::string currentAnimationName; // "idle", "walk", etc.
    int currentFrame;                 // The current frame index (0-9 for idle, etc.)
    sf::Time elapsedTime;             // Time accumulated since the last frame change
    sf::Vector2i frameSize;           // e.g., {32, 32}
};

struct RenderableCircle 
{
    RenderableCircle(float radius, const sf::Color& color, sf::Vector2f position)
        : shape()
    {
        shape.setRadius(radius);
        shape.setFillColor(color);
        shape.setOrigin({ radius, radius });
        shape.setPosition(position);
    }

    ~RenderableCircle() = default;

    sf::CircleShape shape; 
};

struct RenderableRect
{
    RenderableRect(sf::Vector2f size, sf::Color& color, sf::Vector2f position)
        : shape()
    {
        shape.setSize(size);
        shape.setFillColor(color);
        Utils::centerOrigin(shape);
        shape.setPosition(position);
    }

    ~RenderableRect() = default;

    sf::RectangleShape shape;
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