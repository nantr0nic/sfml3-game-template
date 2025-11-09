#pragma once

#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

namespace CoreSystems
{
    //$ ----- Game Systems ----- //
    void handlePlayerInput(entt::registry& registry);

    void movementSystem(entt::registry& registry, sf::Time deltaTime);

    void renderSystem(entt::registry& registry, sf::RenderWindow& window);

    void animationSystem(entt::registry& registry, sf::Time deltaTime);
}

namespace UISystems
{
    //$ ----- UI Systems -----

    void uiRenderSystem(entt::registry& registry, sf::RenderWindow& window);

    void uiClickSystem(entt::registry& registry, const sf::Event::MouseButtonPressed& event);

    void uiHoverSystem(entt::registry& registry, sf::RenderWindow& window);
}