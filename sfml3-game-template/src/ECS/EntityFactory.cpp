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
     * @brief Creates a rectangle-renderable entity and attaches its render component.
     *
     * Creates a new entity in the registry and attaches a RenderableRect component configured
     * with the given size, color, and position.
     *
     * @param context Application context containing the entity registry.
     * @param size Width and height of the rectangle.
     * @param color Fill color for the rectangle.
     * @param position Position of the rectangle's origin in world coordinates.
     * @return entt::entity The created rectangle entity.
     */
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

    /**
     * @brief Creates a UI button entity with a shaped background, centered label, input bounds, and click action.
     *
     * The function creates a new entity in the registry, attaches a tag-specific UI marker component based on
     * `tag` (Menu, Settings, Transition, or Pause), a UIShape sized and positioned at `position`, a centered
     * UIText using `font` and `text`, a UIBounds component derived from the shape, and a UIAction that invokes
     * `action` when activated.
     *
     * @param context Application context containing the entity registry used to create and store the entity.
     * @param font Font used to construct the button label.
     * @param text Label string displayed on the button.
     * @param position World position where the button is placed; both shape and text are centered at this position.
     * @param action Callable invoked when the button is clicked or activated.
     * @param tag Enumeration value selecting which UI tag component to attach (Menu, Settings, Transition, Pause).
     * @param size Size of the button shape (width, height).
     * @return entt::entity The created button entity.
     */
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

    /**
     * @brief Creates a textured GUI button entity and registers its UI components.
     *
     * Constructs a new entity representing a GUI button, assigns a tag-specific UI marker,
     * attaches a GUI button marker, stores the provided texture as a sprite positioned at
     * the given coordinates, records the sprite bounds for hit testing, and associates
     * the provided click action.
     *
     * @param context Application context providing the entity registry.
     * @param texture Texture used to build the button sprite.
     * @param position Position where the button sprite will be placed.
     * @param action Callback invoked when the button is activated.
     * @param tag UI category used to attach a corresponding UI tag component (Menu, Settings, Transition, Pause).
     * @return entt::entity The created button entity.
     */
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

    /**
     * @brief Creates a text label entity positioned to the left of an existing UI button.
     *
     * Creates a new entity tagged for the specified UI layer, attaches a `UIText`
     * component using the provided font, string, size, and color, and positions the
     * label immediately to the left of the target button while vertically centering
     * it relative to the button. The label's text origin is set so the text is
     * right-aligned against the computed position.
     *
     * @param context Application context containing the entity registry.
     * @param buttonEntity The existing button entity whose `UIBounds` are used to position the label.
     * @param font Font used for the label text.
     * @param text String content of the label.
     * @param size Character size for the label text.
     * @param color Fill color for the label text.
     * @param tag UI tag to attach to the label (Menu, Settings, Transition, or Pause).
     * @return entt::entity The newly created label entity.
     */
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

    /**
     * @brief Creates a textured GUI button entity with a right-aligned label positioned to the left of the button.
     *
     * Constructs a button entity that contains a sprite built from the provided texture, a UI action invoked on activation,
     * and a text label aligned to the right and placed to the left of the button sprite with a fixed padding.
     *
     * @param context Application context providing the ECS registry and asset access.
     * @param texture Texture used to create the button sprite.
     * @param position Position of the button sprite in world/window coordinates.
     * @param action Callable invoked when the button is activated (clicked).
     * @param font Font used for the label text.
     * @param tag Logical UI category for the button (Menu, Settings, Transition, Pause).
     * @param text Label string displayed to the left of the button.
     * @param size Character size for the label text.
     * @param color Fill color applied to the label text.
     * @return entt::entity The newly created button entity.
     */
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