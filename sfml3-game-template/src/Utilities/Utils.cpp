#include "Utilities/Utils.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/View.hpp>

#include "Utilities/Logger.hpp"

#include <algorithm>
#include <cstdint>
#include <string_view>

void utils::boxView(sf::View &view, unsigned int windowWidth, unsigned int windowHeight)
{
    if (windowWidth == 0 || windowHeight == 0 || view.getSize().y <= 0.0f)
    {
        logger::Warn("Invalid window or view dimensions! boxView() failed.");
        return;
    }

    float windowRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    float viewRatio = view.getSize().x / view.getSize().y;

    float sizeX = 1;
    float sizeY = 1;
    float posX = 0;
    float posY = 0;

    bool horizontalSpacing = windowRatio >= viewRatio;

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

sf::Color utils::loadColorFromConfig(const ConfigManager& configManager, std::string_view configID,
                                     std::string_view section, std::string_view colorKey)
{
    auto* configTable = configManager.getConfigTable(configID);
    if (!configTable)
    {
        logger::Warn("Invalid config ID! loadColorFromConfig() failed.");
        return sf::Color::Magenta;
    }

    auto* valueColorArray = (*configTable)[section][colorKey].as_array();
    if (valueColorArray && valueColorArray->size() == 3)
        {
            auto r = valueColorArray->at(0).value_or(255LL);
            auto g = valueColorArray->at(1).value_or(0LL);
            auto b = valueColorArray->at(2).value_or(255LL);

            std::uint8_t red = static_cast<std::uint8_t>(std::clamp<long long>(r, 0, 255));
            std::uint8_t green = static_cast<std::uint8_t>(std::clamp<long long>(g, 0, 255));
            std::uint8_t blue = static_cast<std::uint8_t>(std::clamp<long long>(b, 0, 255));

            return sf::Color(red, green, blue);
        }
    return sf::Color::Magenta;
}

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
