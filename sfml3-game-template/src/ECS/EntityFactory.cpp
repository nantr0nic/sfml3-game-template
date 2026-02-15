#include "ECS/EntityFactory.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>
#include <entt/entt.hpp>

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

    /**
     * @brief Creates an entity configured to render a colored rectangle.
     *
     * @return entt::entity The newly created entity; it has a `RenderableRect` component
     *         initialized with the provided size, color, and position.
     */
    entt::entity createRectangle(AppContext& context, 
                                sf::Vector2f size,
                                const sf::Color& color,
                                sf::Vector2f position)
    {
        auto& registry = *context.m_Registry;

        auto rectEntity = registry.create();

        registry.emplace<RenderableRect>(rectEntity, size, color, position);

        return rectEntity;
    }

    //$ ----- G/UI ----- //
    /**
     * @brief Positions a text label to the left of a rectangular UI element and aligns its origin to the label's right-center.
     *
     * Positions `text` so it sits to the left of `buttonRect` with `padding` pixels between the label and the rectangle, and sets the text origin to its right-center to enable correct alignment.
     *
     * @param text Text object to position; its origin will be modified.
     * @param buttonRect Bounds of the target rectangle (position and size) used as the reference for positioning.
     * @param padding Distance in pixels to place between the right edge of the label and the left edge of the rectangle. Default is 10.0f.
     */
    void positionLabelLeftOf(sf::Text& text, const sf::FloatRect& buttonRect, float padding = 10.0f)
    {
        sf::FloatRect textBounds = text.getLocalBounds();

        // set origin to RIGHT-CENTER of the text
        sf::Vector2f origin;
        origin.x = textBounds.position.x + textBounds.size.x; // far right edge
        origin.y = textBounds.position.y + (textBounds.size.y / 2.0f);
        text.setOrigin(origin);

        sf::Vector2f position;
        position.x = buttonRect.position.x - padding; // left of the button
        position.y = buttonRect.position.y + (buttonRect.size.y / 2.0f);

        text.setPosition(position);
    }
    
    /**
     * @brief Creates a clickable rectangular UI button entity with a centered text label.
     *
     * Creates an entity tagged with the provided UITag, attaches a rectangle shape (filled blue)
     * positioned and origin-centered at `position`, a centered text label rendered with `font`,
     * bounds for hit testing, and an action invoked when the button is activated.
     *
     * @param font Font used to render the button label.
     * @param text String displayed on the button.
     * @param position Center position for the button shape and label.
     * @param action Callback invoked when the button is clicked.
     * @param tag Logical UI tag assigned to the created entity.
     * @param size Size of the button shape (width, height).
     * @return entt::entity The created entity handle.
     */
    entt::entity createButton(AppContext& context, sf::Font& font,
                            const std::string& text, sf::Vector2f position,
                            std::function<void()> action,
                            UITags tag, sf::Vector2f size)
    {
        auto& registry = *context.m_Registry;

        auto buttonEntity = registry.create();

        // Tag component
        registry.emplace<UITagID>(buttonEntity, tag);

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

    /**
     * @brief Create an interactive GUI button entity with a textured sprite and click action.
     *
     * The new entity is tagged with `UITagID` and `GUIButtonTag`, and receives `GUISprite`,
     * `UIBounds`, and `UIAction` components configured from the provided texture, position,
     * and callback.
     *
     * @param context Application context providing access to the ECS registry and resources.
     * @param texture Texture used to construct the button's sprite.
     * @param position Position at which the button sprite will be placed.
     * @param action Callback invoked when the button is activated (clicked).
     * @param tag Logical UI tag to assign to the button entity.
     * @return entt::entity The created entity identifier.
     */
    entt::entity createGUIButton(AppContext& context, sf::Texture& texture,
                                sf::Vector2f position,
                                std::function<void()> action, UITags tag)
    {
        auto& registry = *context.m_Registry;
        auto buttonEntity = registry.create();

        // Tag Components
        registry.emplace<UITagID>(buttonEntity, tag);
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

    /**
     * @brief Create a UI text label positioned to the left of an existing button and tag it.
     *
     * Creates an entity with a UIText component rendered with the given font, string,
     * size, and color, positions that text to the left of the provided button's bounds,
     * and attaches a UITagID carrying the specified tag.
     *
     * @param buttonEntity The button entity whose bounds are used to position the label.
     * @param font Font used to render the label text.
     * @param text String to display in the label.
     * @param size Character size for the label text.
     * @param color Fill color for the label text.
     * @param tag UITags value stored in the label's UITagID component.
     * @return entt::entity The created label entity.
     */
    entt::entity createButtonLabel(AppContext& context, const entt::entity buttonEntity,
                                sf::Font& font, const std::string& text,
                                unsigned int size, const sf::Color& color, UITags tag)
    {
        auto& registry = *context.m_Registry;
        auto labelEntity = registry.create();
        
        // Tag component
        registry.emplace<UITagID>(labelEntity, tag); 
    
        // Get the button's bounds
        auto& buttonBounds = registry.get<UIBounds>(buttonEntity);
        
        // Create and style the text
        auto& labelText = registry.emplace<UIText>(labelEntity, sf::Text(font, text, size));
        labelText.text.setFillColor(color);
    
        // Position the label
        positionLabelLeftOf(labelText.text, buttonBounds.rect);
    
        return labelEntity;
    }

    /**
     * @brief Create a GUI button entity composed of a sprite and a left-aligned text label.
     *
     * @param context Application context providing the entity registry and resources.
     * @param texture Texture used to construct the button sprite.
     * @param position World position where the button sprite will be placed.
     * @param action Callback invoked when the button is activated.
     * @param font Font used for the label text.
     * @param tag Logical UI tag assigned to the created entity.
     * @param text String content of the label displayed to the left of the button.
     * @param size Character size for the label text.
     * @param color Fill color for the label text.
     * @return entt::entity The created entity representing the labeled GUI button.
     */
    entt::entity createLabeledButton(AppContext &context, sf::Texture &texture,
                                sf::Vector2f position, std::function<void ()> action,
                                sf::Font& font, UITags tag, const std::string& text,
                                unsigned int size, const sf::Color& color)
    {
        auto& registry = *context.m_Registry;
        auto buttonEntity = registry.create();
    
        // Tag components
        registry.emplace<UITagID>(buttonEntity, tag);
        registry.emplace<GUIButtonTag>(buttonEntity);
    
        // Sprite component
        sf::Sprite buttonSprite(texture);
        buttonSprite.setPosition(position);
        sf::FloatRect bounds = buttonSprite.getGlobalBounds();
        registry.emplace<GUISprite>(buttonEntity, std::move(buttonSprite));
    
        // Button bounds component
        auto& buttonBounds = registry.emplace<UIBounds>(buttonEntity, bounds);
        registry.emplace<UIAction>(buttonEntity, std::move(action));
    
        // Label text
        auto& labelText = registry.emplace<UIText>(buttonEntity, sf::Text(font, text, size));
        labelText.text.setFillColor(color);
    
        // Position the label
        positionLabelLeftOf(labelText.text, buttonBounds.rect);
    
        return buttonEntity;
    }
}