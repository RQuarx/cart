#include <utility>

#include <SDL3/SDL_pixels.h>

#include "terminal/color.hh"

using cart::term::color;


namespace
{
    [[nodiscard]]
    constexpr auto cube_color(std::uint8_t index) noexcept -> std::uint32_t
    {
        static constexpr std::array<std::uint8_t, 6> cube_steps = { 0, 95, 135, 175, 215, 255 };

        const std::uint8_t i = index - 16;
        const std::uint8_t r = i / 36;
        const std::uint8_t g = (i / 6) % 6;
        const std::uint8_t b = i % 6;

        return (std::uint32_t(cube_steps[r]) << 16) | (std::uint32_t(cube_steps[g]) << 8)
             | std::uint32_t(cube_steps[b]);
    }


    [[nodiscard]]
    constexpr auto grayscale_color(std::uint8_t index) noexcept -> std::uint32_t
    {
        const std::uint8_t level = 8 + ((index - 232) * 10);
        return (std::uint32_t(level) << 16) | (std::uint32_t(level) << 8) | std::uint32_t(level);
    }


    [[nodiscard]]
    constexpr auto build_fixed_palette() noexcept -> std::array<std::uint32_t, 256>
    {
        std::array<std::uint32_t, 256> table {};
        for (int i = 16; i < 232; i++) table[i] = cube_color(i);
        for (int i = 232; i < 256; i++) table[i] = grayscale_color(i);
        return table;
    }


    constexpr auto fixed_palette = build_fixed_palette();
}


auto color::resolve(const theme &theme, intensity intensity) const noexcept -> std::uint32_t
{
    auto handle_default = [&](std::array<std::uint32_t, 3> theme::*data)
    { return (theme.*data)[std::to_underlying(intensity)]; };

    auto handle_palette = [&](std::size_t index, enum intensity intensity)
    { return theme.palette[std::to_underlying(intensity)][index]; };

    switch (get_mode())
    {
    case mode::default_foreground: return handle_default(&theme::foreground);
    case mode::default_background: return handle_default(&theme::background);

    case mode::palette:
        {
            const std::uint8_t index = get_palette_index();

            if (index < 8) return handle_palette(index, intensity);
            if (index < 16) return handle_palette(index - 8, intensity::bright);

            return fixed_palette[index];
        }

    case mode::truecolor: return get_rgb();
    }

    std::unreachable();
}
