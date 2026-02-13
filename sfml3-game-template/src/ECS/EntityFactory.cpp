#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

#include "ECS/EntityFactory.hpp"
#include "ECS/Components.hpp"
#include "Utilities/Utils.hpp"
#include "Utilities/Logger.hpp"
#include "AppContext.hpp"
#include "AssetKeys.hpp"

#include <string>
#include <utility>

// functions for the ECS system
namespace EntityFactory
{
    //$ --- Player ---
    entt::entity createPlayer(AppContext& context, sf::Vector2f position)
    {
        auto& registry = *context.m_Registry;
        auto* texture = context.m_ResourceManager->getResource<sf::Texture>(Assets::Textures::Player);

        if (texture == nullptr)
        {
            logger::Error("Couldn't create Player because missing texture.");
            return entt::null;
        }

        context.m_ConfigManager->loadConfig("player", "config/Player.toml");
        float moveSpeed = context.m_ConfigManager->getConfigValue<float>("player", "player", "movementSpeed").value_or(350.0f);
        float scaleFactor = context.m_ConfigManager->getConfigValue<float>("player", "player", "scaleFactor").value_or(3.0f);

        auto playerEntity = registry.create();

        // Add all components that make a "player"
        registry.emplace<PlayerTag>(playerEntity);  // way to ID the player
        registry.emplace<MovementSpeed>(playerEntity, moveSpeed);
        registry.emplace<Velocity>(playerEntity);
        registry.emplace<Facing>(playerEntity);

        // Sprite stuff
        auto& spriteComp = registry.emplace<SpriteComponent>(playerEntity, sf::Sprite(*texture));
        spriteComp.sprite.setTextureRect({ {0, 0}, {32, 32} }); // assumes 32x32 sprite size
        spriteComp.sprite.setPosition(position);
        utils::centerOrigin(spriteComp.sprite);

        // Sprite scaling and padding stuff
        sf::Vector2f scaleVector = { scaleFactor, scaleFactor };

        registry.emplace<BaseScale>(playerEntity, scaleVector);
        // Apply scaling BEFORE getSpritePadding()
        spriteComp.sprite.setScale(scaleVector);
        SpritePadding padding = utils::getSpritePadding(spriteComp.sprite);

        registry.emplace<ConfineToWindow>(
            playerEntity,
            padding.left * scaleFactor,
            padding.right * scaleFactor,
            padding.top * scaleFactor,
            padding.bottom * scaleFactor
        );

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
        logger::Info("Player created.");

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

    //$ ----- G/UI ----- //
    entt::entity createButton(AppContext& context, sf::Font& font,
                            const std::string& text, sf::Vector2f position,
                            std::function<void()> action,
                            UITags tag, sf::Vector2f size)
    {
        auto& registry = *context.m_Registry;

        auto buttonEntity = registry.create();

        switch (tag)
        {
            case UITags::Menu:
                registry.emplace<MenuUITag>(buttonEntity);
                break;
            case UITags::Settings:
                registry.emplace<SettingsUITag>(buttonEntity);
                break;
            case UITags::Transition:
                registry.emplace<TransUITag>(buttonEntity);
                break;
            case UITags::Pause:
                registry.emplace<PauseUITag>(buttonEntity);
                break;
            default:
                break;
        }

        // Shape component
        auto& buttonShape = registry.emplace<UIShape>(buttonEntity);
        buttonShape.shape.setSize(size);
        buttonShape.shape.setFillColor(sf::Color::Blue);
        utils::centerOrigin(buttonShape.shape);
        buttonShape.shape.setPosition(position);

        // Text component
        auto& buttonText = registry.emplace<UIText>(
            buttonEntity,
            sf::Text(font, text, 50)
        );
        utils::centerOrigin(buttonText.text);
        buttonText.text.setPosition(position);
        buttonText.text.setFillColor(sf::Color(200, 200, 200));

        // Bounds component
        registry.emplace<UIBounds>(buttonEntity, buttonShape.shape.getGlobalBounds());

        // Clickable component
        registry.emplace<UIAction>(buttonEntity, std::move(action));

        return buttonEntity;
    }

    entt::entity createGUIButton(AppContext& context, sf::Texture& texture,
                                sf::Vector2f position,
                                std::function<void()> action, UITags tag)
    {
        auto& registry = *context.m_Registry;
        auto buttonEntity = registry.create();

        switch (tag)
        {
            case UITags::Menu:
                registry.emplace<MenuUITag>(buttonEntity);
                break;
            case UITags::Settings:
                registry.emplace<SettingsUITag>(buttonEntity);
                break;
            case UITags::Transition:
                registry.emplace<TransUITag>(buttonEntity);
                break;
            case UITags::Pause:
                registry.emplace<PauseUITag>(buttonEntity);
                break;
            default:
                break;
        }

        registry.emplace<GUIButtonTag>(buttonEntity);

        sf::Sprite buttonSprite(texture);
        buttonSprite.setPosition(position);
        sf::FloatRect bounds = buttonSprite.getGlobalBounds();
        registry.emplace<GUISprite>(buttonEntity, std::move(buttonSprite));

        // Bounds component
        registry.emplace<UIBounds>(buttonEntity, bounds);

        // Clickable component
        registry.emplace<UIAction>(buttonEntity, std::move(action));

        return buttonEntity;
    }

    entt::entity createButtonLabel(AppContext& context, const entt::entity buttonEntity,
                                sf::Font& font, const std::string& text,
                                unsigned int size, const sf::Color& color, UITags tag)
    {
        auto& registry = *context.m_Registry;
        auto labelEntity = registry.create();
        switch (tag)
        {
            case UITags::Menu:
                registry.emplace<MenuUITag>(labelEntity);
                break;
            case UITags::Settings:
                registry.emplace<SettingsUITag>(labelEntity);
                break;
            case UITags::Transition:
                registry.emplace<TransUITag>(labelEntity);
                break;
            case UITags::Pause:
                registry.emplace<PauseUITag>(labelEntity);
                break;
            default:
                break;
        }

        // We'll assume the label goes to the left (for now)
        auto& buttonBounds = registry.get<UIBounds>(buttonEntity);
        sf::FloatRect buttonRect = buttonBounds.rect;

        auto& labelText = registry.emplace<UIText>(labelEntity, sf::Text(font, text, size));
        labelText.text.setFillColor(color);

        sf::FloatRect textBounds = labelText.text.getLocalBounds();

        // set origin to RIGHT-CENTER of the text
        sf::Vector2f origin;
        origin.x = textBounds.position.x + textBounds.size.x; // far right edge
        origin.y = textBounds.position.y + (textBounds.size.y / 2.0f);
        labelText.text.setOrigin(origin);

        float labelPadding = 10.0f;

        sf::Vector2f position;
        position.x = buttonRect.position.x - labelPadding; // left of the button
        position.y = buttonRect.position.y + (buttonRect.size.y / 2.0f);

        labelText.text.setPosition(position);

        return labelEntity;
    }

    entt::entity createLabeledButton(AppContext &context, sf::Texture &texture,
                                sf::Vector2f position, std::function<void ()> action,
                                sf::Font& font, UITags tag, const std::string& text,
                                unsigned int size, const sf::Color& color)
    {
        auto& registry = *context.m_Registry;
        auto buttonEntity = registry.create();

        switch (tag)
        {
            case UITags::Menu:
                registry.emplace<MenuUITag>(buttonEntity);
                break;
            case UITags::Settings:
                registry.emplace<SettingsUITag>(buttonEntity);
                break;
            case UITags::Transition:
                registry.emplace<TransUITag>(buttonEntity);
                break;
            case UITags::Pause:
                registry.emplace<PauseUITag>(buttonEntity);
                break;
            default:
                break;
        }

        registry.emplace<GUIButtonTag>(buttonEntity);

        sf::Sprite buttonSprite(texture);
        buttonSprite.setPosition(position);
        sf::FloatRect bounds = buttonSprite.getGlobalBounds();
        registry.emplace<GUISprite>(buttonEntity, std::move(buttonSprite));

        // Bounds component
        auto& buttonBounds = registry.emplace<UIBounds>(buttonEntity, bounds);
        sf::FloatRect buttonRect = buttonBounds.rect;

        // Clickable component
        registry.emplace<UIAction>(buttonEntity, std::move(action));

        // Label text
        auto& labelText = registry.emplace<UIText>(buttonEntity, sf::Text(font, text, size));
        labelText.text.setFillColor(color);

        sf::FloatRect textBounds = labelText.text.getLocalBounds();

        sf::Vector2f origin;
        origin.x = textBounds.position.x + textBounds.size.x; // far right edge
        origin.y = textBounds.position.y + (textBounds.size.y / 2.0f);
        labelText.text.setOrigin(origin);

        float labelPadding = 10.0f;

        sf::Vector2f labelPosition;
        labelPosition.x = buttonRect.position.x - labelPadding; // left of the button
        labelPosition.y = buttonRect.position.y + (buttonRect.size.y / 2.0f);

        labelText.text.setPosition(labelPosition);

        return buttonEntity;
    }
}