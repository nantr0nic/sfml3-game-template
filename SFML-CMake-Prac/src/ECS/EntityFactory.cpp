#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

#include "ECS/EntityFactory.hpp"
#include "ECS/Components.hpp"
#include "Utils.hpp"
#include "AppContext.hpp"

#include <print>
#include <iostream>
#include <string>

// functions for the ECS system
namespace EntityFactory
{
    //$ --- Player ---
    entt::entity createPlayer(AppContext& context, sf::Vector2f position)
    {
        auto& registry = *context.m_Registry;
        auto* texture = context.m_ResourceManager->getResource<sf::Texture>("PlayerSpriteSheet");

        if (texture == nullptr)
        {
            std::println(std::cerr, "<EntityFactory> Couldn't create Player because missing texture.");
            return entt::null;
        }

        auto playerEntity = registry.create();

        // Add all components that make a "player"
        registry.emplace<PlayerTag>(playerEntity);  // way to ID the player
        registry.emplace<MovementSpeed>(playerEntity, 350.0f);
        registry.emplace<Velocity>(playerEntity);
        registry.emplace<Facing>(playerEntity);
        registry.emplace<BaseScale>(playerEntity, sf::Vector2f(3.0f, 3.0f));

        // Sprite stuff
        sf::Sprite playerSprite(*texture);
        auto& spriteComp = registry.emplace<SpriteComponent>(playerEntity, std::move(playerSprite));
        spriteComp.sprite.setTextureRect({ {0, 0}, {32, 32} }); // assumes 32x32 sprite size
        spriteComp.sprite.setPosition(position);
        Utils::centerOrigin(spriteComp.sprite);

        // Animator stuff
        auto& animator = registry.emplace<AnimatorComponent>(playerEntity);
        
        animator.currentAnimationName = "idle";
        animator.currentFrame = 0;
        animator.elapsedTime = sf::Time::Zero;
        animator.frameSize = { 32, 32 };

        // Define "idle" and "walk" for player
        // (using Brackey's knight sprite sheet)
        // the milliseconds will probably have to be adjusted
        animator.animations["idle"] = { 0, 4, sf::milliseconds(400) };
        animator.animations["walk"] = { 3, 8, sf::milliseconds(800) };

        //registry.emplace<AnimatorComponent>(playerEntity, std::move(animator));
        return playerEntity;
    }

    entt::entity createRectangle(AppContext& context, 
                                sf::Vector2f size,
                                sf::Color& color,
                                sf::Vector2f position)
    {
        auto& registry = *context.m_Registry;

        auto rectEntity = registry.create();

        registry.emplace<RenderableRect>(rectEntity, size, color, position);

        return rectEntity;
    }

    //$ --- UI ---
    entt::entity createButton(AppContext& context,
                            sf::Font& font,
                            const std::string& text,
                            sf::Vector2f position,
                            std::function<void()> action)
    {
        auto& registry = *context.m_Registry;

        auto buttonEntity = registry.create();
        registry.emplace<MenuUITag>(buttonEntity); // Tag for easy cleanup

        // Shape component
        auto& buttonShape = registry.emplace<UIShape>(buttonEntity);
        buttonShape.shape.setSize({200.f, 100.f});
        buttonShape.shape.setFillColor(sf::Color::Blue);
        Utils::centerOrigin(buttonShape.shape);
        buttonShape.shape.setPosition(position);

        // Text component
        auto& buttonText = registry.emplace<UIText>(
            buttonEntity,
            sf::Text(font, text, 50)
        );
        Utils::centerOrigin(buttonText.text);
        buttonText.text.setPosition(position);
        buttonText.text.setFillColor(sf::Color(200, 200, 200));

        // Bounds component
        registry.emplace<Bounds>(buttonEntity, buttonShape.shape.getGlobalBounds());

        // Clickable component
        registry.emplace<Clickable>(buttonEntity, std::move(action));
        
        return buttonEntity;
    }
}