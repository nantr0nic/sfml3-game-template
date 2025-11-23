#include "ECS/Systems.hpp"
#include "ECS/Components.hpp"
#include "AppContext.hpp"

namespace CoreSystems
{
    //$ "Core" / game systems (maybe rename...)
    void handlePlayerInput(AppContext* m_AppContext)
    {
        auto &registry = *m_AppContext->m_Registry;
        auto &window = *m_AppContext->m_MainWindow;

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
            //! temporary way to turn off music while debugging
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::M))
            {
                if (auto music = m_AppContext->m_ResourceManager->getResource<sf::Music>("MainSong"))
                {
                    music->pause();
                }
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

    void movementSystem(entt::registry& registry, sf::Time deltaTime, sf::RenderWindow& window)
    {
        // cache window size
        auto windowSize = window.getSize();

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

                float padLeft = bounds->padLeft;
                float padRight = bounds->padRight;
                float padTop = bounds->padTop;
                float padBottom = bounds->padBottom;

                // West Wall
                if (spriteBounds.position.x + padLeft < 0.0f) 
                {
                    float overlap = (spriteBounds.position.x + padLeft) - 0.0f;
                    spriteComp.sprite.move({ -overlap, 0.0f });
                }
                // East Wall
                if (spriteBounds.position.x + spriteBounds.size.x - padRight > windowSize.x) 
                {
                    float overlap = (spriteBounds.position.x + spriteBounds.size.x - padRight) - windowSize.x;
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

    void renderSystem(entt::registry& registry, sf::RenderWindow& window)
    {
        // now renders anything with a sprite
        auto view = registry.view<SpriteComponent>();
        for (auto entity : view)
        {
            const auto& spriteComp = view.get<SpriteComponent>(entity);
            window.draw(spriteComp.sprite);

            //! temporary: debug bounding box
            sf::RectangleShape debugBox(sf::Vector2f(spriteComp.sprite.getGlobalBounds().size));
            debugBox.setPosition(spriteComp.sprite.getGlobalBounds().position);
            debugBox.setFillColor(sf::Color::Transparent);
            debugBox.setOutlineColor(sf::Color::Red);
            debugBox.setOutlineThickness(1.0f);
            window.draw(debugBox);
        }
    }

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
        auto view = registry.view<Bounds>();

        for (auto entity : view)
        {
            const auto& bounds = view.get<Bounds>(entity);
            if (bounds.rect.contains(mousePos))
            {
                registry.emplace_or_replace<Hovered>(entity);
            }
            else if (registry.all_of<Hovered>(entity))
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