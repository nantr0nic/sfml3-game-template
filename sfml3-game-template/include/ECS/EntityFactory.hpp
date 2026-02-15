#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/System/Vector2.hpp>
#include <entt/entt.hpp>

#include "AppContext.hpp"
#include "Components.hpp"

#include <functional>

namespace EntityFactory
{
    entt::entity createPlayer(AppContext& context, sf::Vector2f position);

    entt::entity createRectangle(AppContext& context,
                                sf::Vector2f size,
                                const sf::Color& color,
                                sf::Vector2f position);
    
    //$ --- G/UI Entities --- //
    entt::entity createButton(AppContext& context,
                            sf::Font& font,
                            const std::string& text,
                            sf::Vector2f position,
                            std::function<void()> action,
                            UITags tag = UITags::Menu,
                            sf::Vector2f size = {250.0f, 100.0f});

    entt::entity createGUIButton(AppContext& context,
                                sf::Texture& texture,
                                sf::Vector2f position,
                                std::function<void()> action,
                                UITags tag = UITags::Menu);

    entt::entity createButtonLabel(AppContext& context,
                                   const entt::entity buttonEntity,
                                   sf::Font& font, const std::string& text,
                                   unsigned int size = 32,
                                   const sf::Color& color = sf::Color::White,
                                   UITags tag = UITags::Menu);

    entt::entity createLabeledButton(AppContext& context, 
                                    sf::Texture& texture,
                                    sf::Vector2f position,
                                    std::function<void()> action,
                                    sf::Font& font,
                                    UITags tag = UITags::Menu,
                                    const std::string& text = "",
                                    unsigned int size = 32,
                                    const sf::Color& color = sf::Color::White);
}