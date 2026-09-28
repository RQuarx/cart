#pragma once
#include <array>
#include <cstdint>


namespace cart::core
{
    struct theme
    {
        std::array<std::uint32_t, 3> foreground { 0xFFFFFF, 0xFFFFFF, 0xE3C7A1 };
        std::array<std::uint32_t, 3> background { 0x404040, 0x000000, 0x000000 };

        std::array<std::array<std::uint32_t, 8>, 3> palette {
            { { 0x404040, 0xFF0000, 0x00FF00, 0xFFFF00, 0x0000FF, 0xFF00FF, 0x00FFFF, 0xFFFFFF },
             { 0x000000, 0xCD0000, 0x00CD00, 0xCDCD00, 0x0000CD, 0xCD00CD, 0x00CDCD, 0xFAEBD7 },
             { 0x000000, 0x872B22, 0x549E4E, 0xAAAB34, 0x0F2353, 0x972596, 0x31A7A6, 0xE3C7A1 } }
        };

        bool bold_as_bright = true;
    };
}
