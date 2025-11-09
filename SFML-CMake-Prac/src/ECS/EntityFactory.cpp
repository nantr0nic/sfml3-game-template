#include "ECS/EntityFactory.hpp"
#include "ECS/Components.hpp"

// functions for the ECS system
namespace EntityFactory
{
    //$ --- Player ---
    entt::entity createPlayer(entt::registry& registry, sf::Vector2f position)
    {
        auto playerEntity = registry.create();

        // Add all components that make a "player"
        registry.emplace<PlayerTag>(playerEntity);  // way to ID the player
        registry.emplace<MovementSpeed>(playerEntity, 200.0f);
        registry.emplace<Velocity>(playerEntity);
        registry.emplace<PlayerCircle>(playerEntity, 50.f, sf::Color::Green, position);
        
        return playerEntity;
    }

    //$ --- UI ---
    entt::entity createButton(entt::registry& registry,
                            sf::Font& font,
                            const std::string& text,
                            sf::Vector2f position,
                            std::function<void()> action)
    {
        auto buttonEntity = registry.create();
        registry.emplace<MenuUITag>(buttonEntity); // Tag for easy cleanup

        // Shape component
        auto& buttonShape = registry.emplace<UIShape>(buttonEntity);
        buttonShape.shape.setSize({200.f, 100.f});
        buttonShape.shape.setFillColor(sf::Color::Blue);
        centerOrigin(buttonShape.shape);
        buttonShape.shape.setPosition(position);

        // Text component
        auto& buttonText = registry.emplace<UIText>(
            buttonEntity,
            sf::Text(font, text, 50)
        );
        centerOrigin(buttonText.text);
        buttonText.text.setPosition(position);
        buttonText.text.setFillColor(sf::Color(200, 200, 200));

        // Bounds component
        registry.emplace<Bounds>(buttonEntity, buttonShape.shape.getGlobalBounds());

        // Clickable component
        registry.emplace<Clickable>(buttonEntity, std::move(action));
        
        return buttonEntity;
    }
}