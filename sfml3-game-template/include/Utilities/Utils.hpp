#pragma once

#include <SFML/Graphics.hpp>

#include "Managers/ConfigManager.hpp"

#include <string_view>

struct SpritePadding
{
    float left{ 0.0f };
    float right{ 0.0f };
    float top{ 0.0f };
    float bottom{ 0.0f };
};

namespace utils
{
    void boxView(sf::View& view, int windowWidth, int windowHeight);
    
    [[nodiscard]] sf::Color loadColorFromConfig(const ConfigManager& configManager,
                                std::string_view configID, std::string_view section, 
                                std::string_view colorKey);

    SpritePadding getSpritePadding(const sf::Sprite& sprite);
    
    // For now this only works with rectangles (I think), I'll extend it to other
    // types if needed later
    template <typename T>
    /**
     * @brief Set the origin of an item to the center of its local bounds.
     *
     * Centers the drawable's origin so positioning and transformations are applied around its geometric center.
     *
     * @tparam T Type that provides `getLocalBounds()` returning `sf::FloatRect` and `setOrigin(const sf::Vector2f&)`.
     * @param item The item whose origin will be centered.
     */
    void centerOrigin(T& item)
    {
        sf::FloatRect bounds = item.getLocalBounds();
        item.setOrigin(bounds.getCenter());
    }
}