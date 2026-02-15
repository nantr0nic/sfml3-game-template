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
     * @brief Creates a renderable rectangle entity with the specified size, color, and position.
     *
     * @param size Width and height of the rectangle.
     * @param color Fill color of the rectangle.
     * @param position Position of the rectangle's origin in world coordinates.
     * @return entt::entity The newly created entity. 
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
     * @brief Positions a text label to the left of a button and vertically centers it.
     *
     * Positions `text` so its right edge sits `padding` pixels to the left of `buttonRect`'s left edge,
     * and aligns the text vertically to the center of `buttonRect`.
     *
     * @param text The sf::Text object to position; its origin will be set to the text's right-center.
     * @param buttonRect The button's global bounds (position and size) used as the reference for placement.
     * @param padding Horizontal space, in pixels, between the text's right edge and the button's left edge.
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
     * @brief Creates a clickable UI button entity with visual and interaction components.
     *
     * Creates a new entity configured with a rectangular shape, centered label text,
     * bounding rectangle for hit testing, a stored click action, and a UITag identifier.
     *
     * @param font Font used for the button label.
     * @param text Label string displayed on the button.
     * @param position Position of the button's center in world coordinates.
     * @param action Function invoked when the button is activated.
     * @param tag Logical UI tag identifying the button's role or group.
     * @param size Size of the button's rectangular shape.
     * @return entt::entity The created button entity.
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
     * Create a GUI button entity with a sprite, clickable bounds, and an associated action.
     *
     * @param context Application context that provides the ECS registry and shared resources.
     * @param texture Texture used to construct the button sprite.
     * @param position World position where the button sprite will be placed.
     * @param action Callable invoked when the button is activated (e.g., clicked).
     * @param tag UI tag used to categorize or identify the button.
     * @return entt::entity The created GUI button entity.
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
     * @brief Creates a text label entity and positions it to the left of an existing button.
     *
     * Attaches a UITagID and a UIText component with the provided font, string, size, and color,
     * then positions the text to the left of the given button's bounds.
     *
     * @param context Application context supplying the ECS registry.
     * @param buttonEntity Entity of the button whose bounds determine the label position.
     * @param font Font used for the label text.
     * @param text String content for the label.
     * @param size Character size for the label text.
     * @param color Fill color for the label text.
     * @param tag UI tag to assign to the created label entity.
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
     * @brief Create a GUI button entity with a sprite and a label positioned to the left of the sprite.
     *
     * The created entity receives components identifying it as a GUI button, storing its sprite,
     * bounds, click action, and a text label that is positioned left of the button sprite.
     *
     * @param context Application context containing the ECS registry.
     * @param texture Texture used to construct the button sprite.
     * @param position Position at which the button sprite is placed (sprite's local origin is used).
     * @param action Callback invoked when the button is activated; stored on the entity.
     * @param font Font used for the label text.
     * @param tag UI tag value assigned to the button (stored in `UITagID`).
     * @param text Label string displayed next to the button.
     * @param size Character size for the label text.
     * @param color Fill color for the label text.
     * @return entt::entity The entity created for the labeled GUI button (has UITagID, GUIButtonTag, GUISprite, UIBounds, UIAction, and UIText components).
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