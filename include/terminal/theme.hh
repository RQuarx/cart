#pragma once
#include <array>
#include <cstdint>


namespace cart::term
{
    struct theme
    {
        std::array<std::uint32_t, 3>                foreground;
        std::array<std::uint32_t, 3>                background;
        std::array<std::array<std::uint32_t, 8>, 3> palette;
        bool                                        bold_as_bright;
    };
}
