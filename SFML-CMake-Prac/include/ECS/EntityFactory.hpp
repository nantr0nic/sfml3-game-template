#pragma once

#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

#include <functional>

struct AppContext; // forward declaration

namespace EntityFactory
{
    entt::entity createPlayer(AppContext& context, sf::Vector2f position);

    entt::entity createRectangle(AppContext& context,
                                sf::Vector2f size,
                                sf::Color& color,
                                sf::Vector2f position);

    entt::entity createButton(AppContext& context,
                            sf::Font& font,
                            const std::string& text,
                            sf::Vector2f position,
                            std::function<void()> action);
}