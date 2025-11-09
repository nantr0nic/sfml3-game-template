#pragma once

#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

#include <functional>

namespace EntityFactory
{
    entt::entity createPlayer(entt::registry& registry, sf::Vector2f position);

    entt::entity createButton(entt::registry& registry,
                            sf::Font& font,
                            const std::string& text,
                            sf::Vector2f position,
                            std::function<void()> action);
}