#pragma once

#include <string_view>

namespace Assets
{
    namespace Fonts
    {
        constexpr std::string_view MainFont = "MainFont";
        constexpr std::string_view ScoreFont = "ScoreFont";
    }
    namespace Textures
    {
        // Player Sprite
        constexpr std::string_view Player = "PlayerSpriteSheet";
        // UI
        constexpr std::string_view ButtonRedX = "ButtonRedX";
        constexpr std::string_view ButtonLeftArrow = "ButtonLeftArrow";
        constexpr std::string_view ButtonRightArrow = "ButtonRightArrow";
        constexpr std::string_view ButtonBackground = "ButtonBackground";
    }
    namespace SoundBuffers
    {
    }
    namespace Musics
    {
        constexpr std::string_view MainSong = "MainSong";
    }
    namespace Configs
    {
        constexpr std::string_view Window = "WindowConfig";
        constexpr std::string_view Player = "Player";
    }
}
