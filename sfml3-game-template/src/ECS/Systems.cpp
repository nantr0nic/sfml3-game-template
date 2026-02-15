#include "ECS/Systems.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <entt/entt.hpp>

#include "ECS/Components.hpp"
#include "Utilities/Utils.hpp"
#include "AppContext.hpp"
#include "AssetKeys.hpp"

namespace CoreSystems
{
    /**
     * @brief Processes player keyboard input and updates player movement, facing, and animation state.
     *
     * For every entity with PlayerTag, Velocity, MovementSpeed, AnimatorComponent, SpriteComponent, and Facing,
     * this system sets the entity's velocity according to W/S/A/D keys, updates the facing direction for A/D,
     * and switches the animator between "walk" and "idle", resetting the animation frame and elapsed time when the
     * animation name changes.
     *
     * @param m_AppContext Application context containing the registry and main window used for input and entity access.
     */
    void handlePlayerInput(AppContext& m_AppContext)
    {
        auto &registry = *m_AppContext.m_Registry;
        auto &window = *m_AppContext.m_MainWindow;

        auto view = registry.view<PlayerTag,
                                Velocity,
                                MovementSpeed,
                                AnimatorComponent,
                                SpriteComponent,
                                Facing>();

        for (auto entity : view)
        {
            auto& velocity = view.get<Velocity>(entity);
            const auto& speed = view.get<MovementSpeed>(entity);
            auto& animator = view.get<AnimatorComponent>(entity);
            auto& facing = view.get<Facing>(entity);

            // Reset velocity
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
                facing.dir = FacingDirection::Left;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::D))
            {
                velocity.value.x += speed.value;
                facing.dir = FacingDirection::Right;
            }

            //$ --- Animating the player sprite ---
            // if player is walking
            if (velocity.value.x != 0.0f || velocity.value.y != 0.0f)
            {
                // this prevents resetting the animation every frame
                if (animator.currentAnimationName != "walk")
                {
                    animator.currentAnimationName = "walk";
                    animator.currentFrame = 0;
                    animator.elapsedTime = sf::Time::Zero;
                }
            }
            else
            {
                if (animator.currentAnimationName != "idle")
                {
                    animator.currentAnimationName = "idle";
                    animator.currentFrame = 0;
                    animator.elapsedTime = sf::Time::Zero;
                }
            }
        }
    }

    /**
     * @brief Moves entities with Velocity by their velocity scaled by the frame time and constrains them inside the render window when requested.
     *
     * Moves each entity that has a SpriteComponent and Velocity by velocity * deltaTime. If an entity has a ConfineToWindow component, its sprite position is adjusted so its padded bounds remain inside the given window; horizontal padding is applied taking horizontal sprite flipping into account.
     *
     * @param deltaTime Time elapsed since the last update used to scale movement.
     * @param window Render window whose view size is used as the confinement rectangle.
     */
    void movementSystem(entt::registry& registry, sf::Time deltaTime, sf::RenderWindow& window)
    {
        // cache window size
        auto windowSize = window.getView().getSize();

        // now moves anything with a sprite
        auto view = registry.view<SpriteComponent, Velocity>();
        for (auto entity : view)
        {
            auto& spriteComp = view.get<SpriteComponent>(entity);
            const auto& velocity = view.get<Velocity>(entity);

            spriteComp.sprite.move(velocity.value * deltaTime.asSeconds());

            // Check for 'ConfineToWindow' and pad appropriately
            if (auto* bounds = registry.try_get<ConfineToWindow>(entity))
            {
                auto spriteBounds = spriteComp.sprite.getGlobalBounds();

                /* We need this 'isFlipped' because at present we're using a right-facing only
                sprite sheet and we flip the sprite with a negative scale to make it face left.
                So if you/we have both right and left facing sprites in our sheet and use those
                this check will not be necessary (it won't run anyway is scale is > 0). */

                // Check sprite/entity orientation
                // if scale.x is negative, the sprite is flipped horizontally
                bool isFlipped = spriteComp.sprite.getScale().x < 0;

                // Swap horizontal padding if flipped
                float currentPadLeft = isFlipped ? bounds->padRight : bounds->padLeft;
                float currentPadRight = isFlipped ? bounds->padLeft : bounds->padRight;

                // Don't need to swap cuz we're not flipping the sprite along Y axis (in current case)
                float padTop = bounds->padTop;
                float padBottom = bounds->padBottom;

                // West Wall
                if (spriteBounds.position.x + currentPadLeft < 0.0f)
                {
                    float overlap = (spriteBounds.position.x + currentPadLeft) - 0.0f;
                    spriteComp.sprite.move({ -overlap, 0.0f });
                }
                // East Wall
                if (spriteBounds.position.x + spriteBounds.size.x - currentPadRight > windowSize.x)
                {
                    float overlap = (spriteBounds.position.x + spriteBounds.size.x - currentPadRight) - windowSize.x;
                    spriteComp.sprite.move({ -overlap, 0.0f });
                }
                // North Wall
                if (spriteBounds.position.y + padTop < 0.0f)
                {
                    float overlap = (spriteBounds.position.y + padTop) - 0.0f;
                    spriteComp.sprite.move({ 0.0f, -overlap });
                }
                // South Wall
                if (spriteBounds.position.y + spriteBounds.size.y - padBottom > windowSize.y)
                {
                    float overlap = (spriteBounds.position.y + spriteBounds.size.y - padBottom) - windowSize.y;
                    spriteComp.sprite.move({ 0.0f, -overlap });
                }
            }
        }
    }

    void facingSystem(entt::registry& registry)
    {
        auto view = registry.view<SpriteComponent, Facing, BaseScale>();

        for (auto entity : view)
        {
            const auto& facing = view.get<Facing>(entity);
            auto& spriteComp = view.get<SpriteComponent>(entity);
            const auto& baseScale = view.get<BaseScale>(entity);

            if (facing.dir == FacingDirection::Left)
            {
                spriteComp.sprite.setScale({ -baseScale.value.x, baseScale.value.y });
            }
            // this will need else if blocks for up / down or isometric 8 direction facing
            else // facing right
            {
                spriteComp.sprite.setScale(baseScale.value);
            }
        }
    }

    /**
     * @brief Renders all entities that have a SpriteComponent and optionally overlays debug visuals.
     *
     * When debug is enabled, draws each sprite's global bounding box in red and, for entities with a
     * ConfineToWindow component, draws the inner "solid" body rectangle in green.
     *
     * @param registry EnTT registry containing the entities and their components.
     * @param window Render target used to draw sprites and debug shapes.
     * @param showDebug If true, render bounding boxes and confine-area overlays for each sprite.
     */
    void renderSystem(entt::registry& registry, sf::RenderWindow& window, bool showDebug)
    {
        // now renders anything with a sprite
        auto view = registry.view<SpriteComponent>();
        for (auto entity : view)
        {
            const auto& spriteComp = view.get<SpriteComponent>(entity);
            window.draw(spriteComp.sprite);

            if (showDebug)
            {
                auto bounds = spriteComp.sprite.getGlobalBounds();

                //$ Debug: bounding box (red)
                sf::RectangleShape debugBox(bounds.size);
                debugBox.setPosition(bounds.position);
                debugBox.setFillColor(sf::Color::Transparent);
                debugBox.setOutlineColor(sf::Color::Red);
                debugBox.setOutlineThickness(1.0f);
                window.draw(debugBox);

                if (auto* confine = registry.try_get<ConfineToWindow>(entity))
                {
                    // Draw a Green box representing the "Solid" body (inner bounds)
                    sf::RectangleShape solidBox;
                    solidBox.setPosition({
                        bounds.position.x + confine->padLeft,
                        bounds.position.y + confine->padTop
                    });
                    solidBox.setSize({
                        bounds.size.x - (confine->padLeft + confine->padRight),
                        bounds.size.y - (confine->padTop + confine->padBottom)
                    });
                    solidBox.setFillColor(sf::Color::Transparent);
                    solidBox.setOutlineColor(sf::Color::Green);
                    solidBox.setOutlineThickness(1.0f);
                    window.draw(solidBox);
                }
            }
        }
    }

    /**
     * @brief Advances sprite animations for entities that have AnimatorComponent and SpriteComponent.
     *
     * Advances each animator's elapsed time by deltaTime, steps the current frame when enough time
     * has accumulated (wrapping to the first frame after the last), and updates the sprite's texture
     * rectangle to the new frame position.
     *
     * @param registry The EnTT registry containing entities with SpriteComponent and AnimatorComponent.
     * @param deltaTime Time elapsed since the last update frame; used to advance animation timers.
     */
    void animationSystem(entt::registry& registry, sf::Time deltaTime)
    {
        auto view = registry.view<SpriteComponent, AnimatorComponent>();

        for (auto entity : view)
        {
            auto& spriteComp = view.get<SpriteComponent>(entity);
            auto& animator = view.get<AnimatorComponent>(entity);

            // Get the data for the current animation from the map in AnimatorComponent
            Animation& currentAnim = animator.animations.at(animator.currentAnimationName);

            // Add the frame time to our elapsed timer
            animator.elapsedTime += deltaTime;

            // Check if we should advance to the next frame
            if (animator.elapsedTime >= currentAnim.getTimePerFrame())
            {
                // Reset the timer
                animator.elapsedTime -= currentAnim.getTimePerFrame();

                // Advance to the next frame, loop back to 0 if at end
                animator.currentFrame = (animator.currentFrame + 1) % currentAnim.frames;

                // Calculate where to move the frame to on the sprite sheet
                sf::IntRect newRect;
                newRect.position.x = animator.currentFrame * animator.frameSize.x;
                newRect.position.y = currentAnim.row * animator.frameSize.y;
                newRect.size.x = animator.frameSize.x;
                newRect.size.y = animator.frameSize.y;

                // Set the sprite's texture rect
                spriteComp.sprite.setTextureRect(newRect);
            }
        }
    }
}

namespace UISystems
{
    //$ --- UI Systems Implementation ---

    void uiHoverSystem(entt::registry& registry, sf::RenderWindow& window)
    {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        auto view = registry.view<UIBounds>();

        for (auto entity : view)
        {
            const auto& bounds = view.get<UIBounds>(entity);
            if (bounds.rect.contains(mousePos))
            {
                registry.emplace_or_replace<UIHover>(entity);
            }
            else if (registry.all_of<UIHover>(entity))
            {
                registry.remove<UIHover>(entity);
            }
        }
    }

    /**
     * @brief Render UI elements and adjust their visuals based on hover state.
     *
     * Renders UI shapes, text, GUI sprites (buttons), and Red X overlays to the provided window.
     * Shapes change fill color when hovered. Text color is adjusted on hover only for interactive
     * UI entries that have both `UIAction` and `UIBounds`. All GUI sprites and any `GUIRedX`
     * overlays are drawn unconditionally.
     */
    void uiRenderSystem(entt::registry& registry, sf::RenderWindow& window)
    {
        // Render shapes
        auto shapeView = registry.view<UIShape>();
        for (auto shapeEntity : shapeView)
        {
            auto& uiShape = shapeView.get<UIShape>(shapeEntity);

            // Change color on hover
            if (registry.all_of<UIHover>(shapeEntity))
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
        for (auto textEntity : textView)
        {
            auto& uiText = textView.get<UIText>(textEntity);

            // Change text color on hover for interactive UI
            if (registry.all_of<UIAction, UIBounds>(textEntity))
            {
                if (registry.all_of<UIHover>(textEntity))
                {
                    uiText.text.setFillColor(sf::Color::White); // Hover text color
                }
                else
                {
                    uiText.text.setFillColor(sf::Color(200, 200, 200)); // Normal text color
                }
            }

            window.draw(uiText.text);
        }

        // Render UI buttons
        auto buttonView = registry.view<GUISprite>();
        for (auto buttonEntity : buttonView)
        {
            auto& button = buttonView.get<GUISprite>(buttonEntity);
            window.draw(button.sprite);
        }

        // Render Red X overlay
        auto xView = registry.view<GUIRedX>();
        for (auto entity : xView)
        {
            auto& redX = xView.get<GUIRedX>(entity);
            window.draw(redX.sprite);
        }
    }

    /**
     * @brief Invoke click actions for hovered UI elements when the left mouse button is pressed.
     *
     * Calls the callable stored in `UIAction::action` for every entity that currently has both
     * `UIHover` and `UIAction` if the provided mouse event indicates the left button was pressed.
     *
     * @param registry The entity registry containing UI components.
     * @param event The mouse button press event; only `sf::Mouse::Button::Left` triggers actions.
     */
    void uiClickSystem(entt::registry& registry, const sf::Event::MouseButtonPressed& event)
    {
        if (event.button == sf::Mouse::Button::Left)
        {
            auto view = registry.view<UIHover, UIAction>();
            for (auto entity : view)
            {
                auto& clickable = view.get<UIAction>(entity);
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

    /**
     * @brief Ensure red‑X overlay components are present or removed for GUI buttons based on their toggle condition.
     *
     * If the ButtonRedX texture is unavailable the function returns immediately. For each entity with a GUISprite and
     * UIToggleCond, the function will emplace a GUIRedX component containing a red‑X sprite positioned at the button's
     * center when UIToggleCond::shouldShowOverlay() returns true, and remove an existing GUIRedX component when it
     * returns false.
     *
     * @param context Application context used to access the resource manager and the ECS registry.
     */
    void uiSettingsChecks(AppContext& context)
    {
        auto* buttonRedX = context.m_ResourceManager->getResource<sf::Texture>(
                                                                Assets::Textures::ButtonRedX);
        if (!buttonRedX)
        {
            return;
        }

        auto redXSprite = sf::Sprite(*buttonRedX);
        utils::centerOrigin(redXSprite);

        auto& registry = context.m_Registry;

        auto buttonView = registry->view<GUISprite, UIToggleCond>();
        for (auto buttonEntity : buttonView)
        {
            auto& condition = buttonView.get<UIToggleCond>(buttonEntity);
            if (condition.shouldShowOverlay())
            {
                if (!registry->all_of<GUIRedX>(buttonEntity))
                {
                    auto& buttonSprite = registry->get<GUISprite>(buttonEntity);
                    auto buttonCenter = buttonSprite.sprite.getGlobalBounds().getCenter();
                    redXSprite.setPosition(buttonCenter);
                    registry->emplace<GUIRedX>(buttonEntity, redXSprite);
                }
            }
            else
            {
                if (registry->all_of<GUIRedX>(buttonEntity))
                {
                    registry->remove<GUIRedX>(buttonEntity);
                }
            }
        }
    }
}
