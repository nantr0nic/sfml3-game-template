#pragma once

#include <SFML/Graphics.hpp>

namespace Utils
{
    // For now this only works with rectangles, I'll extend it to other
    // types if needed later
    template <typename T>
    void centerOrigin(T& item)
    {
        sf::FloatRect bounds = item.getLocalBounds();
        item.setOrigin(bounds.getCenter());
    }
}