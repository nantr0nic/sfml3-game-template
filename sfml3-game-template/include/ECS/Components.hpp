#pragma once

#include <SFML/Graphics.hpp>

#include "Utilities/Utils.hpp"

#include <functional>
#include <map>
#include <string>

//$ ----- Game Components ----- //

struct PlayerTag {}; // Tag to identify the player entity

struct Velocity { sf::Vector2f value{ 0.0f, 0.0f }; };

struct MovementSpeed { float value{ 0.0f }; };

struct BoundaryHits
{
    bool north{ false };
    bool south{ false };
    bool west{ false };
    bool east{ false };
};

struct ConfineToWindow 
{
    float padLeft{ 0.0f };
    float padRight{ 0.0f };
    float padTop{ 0.0f };
    float padBottom{ 0.0f };
};


// ----- Sprite / Animation Components ----- //
struct SpriteComponent { sf::Sprite sprite; };

struct Animation
{
    Animation() = default;
    Animation(int row, int frames, sf::Time duration)
        : row(row)
        , frames(frames)
        , duration(duration)
    { }

    int row{ 0 };                        // The row on the sprite sheet (0 for idle, 1 for walk, etc.)
    int frames{ 0 };                     // Total number of frames in this animation
    sf::Time duration{ sf::Time::Zero }; // Total duration this animation should take
    
    // Calculates time per frame. sf::Time / float is a valid operation.
    sf::Time getTimePerFrame() const
    { 
        return (frames > 0) ? duration / static_cast<float>(frames) : sf::Time::Zero;
    }
};

struct AnimatorComponent
{
    // Stores all available animations by name 
    std::map<std::string, Animation> animations;

    std::string currentAnimationName{ "" }; // "idle", "walk", etc.
    int currentFrame{ 0 };                      // The current frame index (0-9 for idle, etc.)
    sf::Time elapsedTime{ sf::Time::Zero };     // Time accumulated since the last frame change
    sf::Vector2i frameSize{ 0, 0 };             // e.g., {32, 32}
};

enum class FacingDirection { Left, Right };

struct Facing { FacingDirection dir = FacingDirection::Right; };

struct BaseScale { sf::Vector2f value{ 1.0f, 1.0f }; };

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

    sf::CircleShape shape; 
};

struct RenderableRect
{
    RenderableRect(sf::Vector2f size, sf::Color& color, sf::Vector2f position)
        : shape()
    {
        shape.setSize(size);
        shape.setFillColor(color);
        utils::centerOrigin(shape);
        shape.setPosition(position);
    }

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