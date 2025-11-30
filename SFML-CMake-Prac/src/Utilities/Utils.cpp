#include <SFML/Graphics.hpp>

#include "Utilities/Utils.hpp"

#include <algorithm>

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