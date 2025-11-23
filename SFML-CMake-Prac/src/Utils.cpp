#include <SFML/Graphics.hpp>

#include "Utils.hpp"

SpritePadding Utils::getSpritePadding(const sf::Sprite& sprite)
{
    const sf::Texture& texture = sprite.getTexture();
    
    // This is expensive -- copies texture from GPU to RAM, do not put this into loop
    sf::Image image = texture.copyToImage();

    unsigned int width = image.getSize().x;
    unsigned int height = image.getSize().y;
    
    unsigned int minX = width;
    unsigned int maxX = 0;
    unsigned int minY = height;
    unsigned int maxY = 0;

    bool foundVisible = false;

    // Single pass over all pixels
    for (unsigned int y = 0; y < height; ++y)
    {
        for (unsigned int x = 0; x < width; ++x)
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

    // Calculate padding relative to the full texture size
    // Right Padding = Total Width - (Last Visible Pixel Index) - 1
    return {
        static_cast<float>(minX),                  // Left
        static_cast<float>(width - maxX - 1),      // Right
        static_cast<float>(minY),                  // Top
        static_cast<float>(height - maxY - 1)      // Bottom
    };
}