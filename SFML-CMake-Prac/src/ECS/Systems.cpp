#include "ECS/Systems.hpp"
#include "ECS/Components.hpp"

namespace CoreSystems
{
    //$ "Core" / game systems (maybe rename...)
    void handlePlayerInput(entt::registry& registry)
    {
        auto view = registry.view<PlayerTag, Velocity, MovementSpeed>();
        for (auto entity : view)
        {
            auto& velocity = view.get<Velocity>(entity);
            const auto& speed = view.get<MovementSpeed>(entity);

            velocity.value = { 0.0f, 0.0f };

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::W))
            {
                velocity.value.y -= speed.value;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::S))
            {
                velocity.value.y += speed.value;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::A))
            {
                velocity.value.x -= speed.value;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::D))
            {
                velocity.value.x += speed.value;
            }
        }
    }

    void movementSystem(entt::registry& registry, sf::Time deltaTime)
    {
        // only moves the player for now
        auto view = registry.view<RenderableCircle, Velocity>();
        for (auto entity : view)
        {
            const auto& velocity = view.get<Velocity>(entity);
            auto& renderable = view.get<RenderableCircle>(entity);
            renderable.shape.move(velocity.value * deltaTime.asSeconds());
        }
    }

    void renderSystem(entt::registry& registry, sf::RenderWindow& window)
    {
        // only renders the player for now
        auto view = registry.view<RenderableCircle>();
        for (auto entity : view)
        {
            const auto& renderable = view.get<RenderableCircle>(entity);
            window.draw(renderable.shape);
        }
    }
}

namespace UISystems
{
    //$ --- UI Systems Implementation ---

    void uiHoverSystem(entt::registry& registry, sf::RenderWindow& window)
    {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        auto view = registry.view<Bounds>();

        for (auto entity : view)
        {
            const auto& bounds = view.get<Bounds>(entity);
            if (bounds.rect.contains(mousePos))
            {
                registry.emplace_or_replace<Hovered>(entity);
            }
            else
            {
                registry.remove<Hovered>(entity);
            }
        }
    }

    void uiRenderSystem(entt::registry& registry, sf::RenderWindow& window)
    {
        // Render shapes
        auto shapeView = registry.view<UIShape>();
        for (auto entity : shapeView)
        {
            auto& uiShape = shapeView.get<UIShape>(entity);
            
            // Change color on hover
            if (registry.all_of<Hovered>(entity))
            {
                uiShape.shape.setFillColor(sf::Color(100, 100, 255)); // Hover color
            }
            else
            {
                uiShape.shape.setFillColor(sf::Color::Blue); // Normal color
            }
            
            window.draw(uiShape.shape);
        }

        // Render text
        auto textView = registry.view<UIText>();
        for (auto entity : textView)
        {
            auto& uiText = textView.get<UIText>(entity);

            // Change text color on hover
            if (registry.all_of<Hovered>(entity))
            {
                uiText.text.setFillColor(sf::Color::White); // Hover text color
            }
            else
            {
                uiText.text.setFillColor(sf::Color(200, 200, 200)); // Normal text color
            }

            window.draw(uiText.text);
        }
    }

    void uiClickSystem(entt::registry& registry, const sf::Event::MouseButtonPressed& event)
    {
        if (event.button == sf::Mouse::Button::Left)
        {
            auto view = registry.view<Hovered, Clickable>();
            for (auto entity : view)
            {
                auto& clickable = view.get<Clickable>(entity);
                if (clickable.action)
                {
                    clickable.action();
                }
            }
        }
        else
        {
            // empty
        }
    }
}