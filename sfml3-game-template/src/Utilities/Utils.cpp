#include "Utilities/Utils.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/View.hpp>

#include <algorithm>
#include <cstdint>
#include <string_view>

/**
 * @brief Adjusts a view's viewport to preserve its aspect ratio by adding pillarbox or letterbox bars.
 *
 * Computes the relative viewport rectangle so the view content maintains its aspect ratio within a window
 * of the given width and height, centering the view and leaving empty bars on the sides (pillarbox)
 * or top/bottom (letterbox) as required, then applies it with view.setViewport(...).
 *
 * @param view The SFML view to modify.
 * @param windowWidth Current window width in pixels.
 * @param windowHeight Current window height in pixels.
 */
void utils::boxView(sf::View &view, int windowWidth, int windowHeight)
{
    float windowRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    float viewRatio = view.getSize().x / view.getSize().y;

    float sizeX = 1;
    float sizeY = 1;
    float posX = 0;
    float posY = 0;

    bool horizontalSpacing = true;
    if (windowRatio < viewRatio)
    {
        horizontalSpacing = false;
    }

    // If window is wider than game, add bars on sides (Pillarbox)
    if (horizontalSpacing)
    {
        sizeX = viewRatio / windowRatio;
        posX = (1 - sizeX) / 2.f;
    }
    // If window is taller than game, add bars on top/bottom (Letterbox)
    else
    {
        sizeY = windowRatio / viewRatio;
        posY = (1 - sizeY) / 2.f;
    }

    view.setViewport(sf::FloatRect({posX, posY}, {sizeX, sizeY}));
}

/**
 * @brief Load an RGB color from a configuration table.
 *
 * Retrieves a 3-element numeric array located at [section][colorKey] in the configuration
 * identified by configID and constructs an `sf::Color` from the values.
 *
 * @param configManager Configuration manager used to look up the table.
 * @param configID Identifier of the configuration table to read.
 * @param section Section/key grouping inside the configuration table.
 * @param colorKey Key whose value is expected to be a 3-element array: [R, G, B].
 * @return sf::Color Color constructed from the three array values (`R`, `G`, `B`).
 *         Returns `sf::Color::Magenta` if the config table, array, or array size is missing or invalid.
 */
sf::Color utils::loadColorFromConfig(const ConfigManager& configManager, std::string_view configID,
                                     std::string_view section, std::string_view colorKey)
{
    auto* configTable = configManager.getConfigTable(configID);
    if (!configTable) return sf::Color::Magenta;

    auto* valueColorArray = (*configTable)[section][colorKey].as_array();
    if (valueColorArray && valueColorArray->size() == 3)
    {
        std::uint8_t red = static_cast<uint8_t>(valueColorArray->at(0).value_or(255));
        std::uint8_t green = static_cast<uint8_t>(valueColorArray->at(1).value_or(0));
        std::uint8_t blue = static_cast<uint8_t>(valueColorArray->at(2).value_or(255));
        return sf::Color(red, green, blue);
    }
    return sf::Color::Magenta;
}

/**
 * @brief Computes the pixel padding around the visible (non-transparent) area of a sprite within its texture rectangle.
 *
 * The padding values are measured relative to the sprite's texture rectangle: how many pixels of transparent
 * space exist on the left, right, top, and bottom sides before the first/last visible pixel. If the texture
 * rectangle is invalid or the sprite is completely transparent, an empty (zero) padding is returned.
 *
 * @param sprite Sprite whose texture rect will be analyzed.
 * @return SpritePadding Structure containing `{ left, right, top, bottom }` padding values in pixels as floats.
 */
SpritePadding utils::getSpritePadding(const sf::Sprite& sprite)
{
    const sf::Texture& texture = sprite.getTexture();

    sf::IntRect rect = sprite.getTextureRect();

    // Safety check: Texture might be smaller than the rect (though unlikely)
    if (rect.position.x < 0 || rect.position.y < 0 || rect.size.x <= 0 || rect.size.y <= 0)
    {
        return {};
    }

    // This is expensive -- copies texture from GPU to RAM, do not put this into loop
    sf::Image image = texture.copyToImage();

    // Make sure we don't read out of the image's bounds
    unsigned int startX = static_cast<unsigned int>(rect.position.x);
    unsigned int startY = static_cast<unsigned int>(rect.position.y);
    unsigned int endX = std::min(image.getSize().x, startX + static_cast<unsigned int>(rect.size.x));
    unsigned int endY = std::min(image.getSize().y, startY + static_cast<unsigned int>(rect.size.y));

    // Init mix/max to opposites
    unsigned int minX = endX;
    unsigned int maxX = startX;
    unsigned int minY = endY;
    unsigned int maxY = startY;

    bool foundVisible = false;

    // Single pass over all pixels
    for (unsigned int y = startY; y < endY; ++y)
    {
        for (unsigned int x = startX; x < endX; ++x)
        {
            // Check if pixel is NOT fully transparent
            if (image.getPixel({ x, y }).a > 0)
            {
                if (x < minX)
                {
                    minX = x;
                }
                if (x > maxX)
                {
                    maxX = x;
                }
                if (y < minY)
                {
                    minY = y;
                }
                if (y > maxY)
                {
                    maxY = y;
                }
                foundVisible = true;
            }
        }
    }

    // If the sprite is completely invisible, return 0 padding
    if (!foundVisible)
    {
        return {};
    }

    // Calculate padding relative to the rect of the sprite
    return {
        static_cast<float>(minX - startX),      // Left
        static_cast<float>(endX - maxX - 1),    // Right
        static_cast<float>(minY - startY),      // Top
        static_cast<float>(endY - maxY - 1)     // Bottom
    };
}