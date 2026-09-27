#include <SDL3/SDL_pixels.h>

#include "color.hh"

using cart::color;


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


    [[nodiscard]]
    auto int32_to_fcolor(std::uint32_t val) noexcept -> cart::sdl::fcolor
    {
        cart::sdl::fcolor res;
        res.r = float((val >> 16) & 0xFF) / 255.0F;
        res.g = float((val >> 8) & 0xFF) / 255.0F;
        res.b = float(val & 0xFF) / 255.0F;
        res.a = 1.0F;
        return res;
    }


    [[nodiscard]]
    auto read_hex_entry(const toml::array &arr, std::size_t idx, std::string_view ctx)
        -> std::expected<std::uint32_t, cart::error>
    {
        const auto *node = arr.get(idx);
        if (node == nullptr) [[unlikely]]
            return cart::error { "Index {} missing in \"{}\" colour array.", idx, ctx }
                .unexpected();

        if (!node->is_integer()) [[unlikely]]
            return cart::error { "Entry {} in \"{}\" is not of type `integer`.", idx, ctx }
                .unexpected();

        const std::int64_t hex = node->as_integer()->get();

        if (hex > 0xFFFFFF or hex < 0) [[unlikely]]
            return cart::error { "Value at index {} in \"{}\" does not fit a 24-bit (rgb) colour.",
                                 idx, ctx }
                .unexpected();

        return std::uint32_t(hex);
    }


    [[nodiscard]]
    constexpr auto adjust_color_brightness(cart::sdl::fcolor color, float factor) noexcept
        -> cart::sdl::fcolor
    {
        return { std::min(1.F, color.r * factor), std::min(1.F, color.g * factor),
                 std::min(1.F, color.b * factor), color.a };
    }


    constexpr auto fixed_palette = build_fixed_palette();
}


auto color::get_fcolor(const config &config, intensity intensity) const noexcept
    -> std::expected<sdl::fcolor, error>
{
    switch (get_mode())
    {
    case mode::default_foreground: return handle_default_color(config, "foreground", intensity);
    case mode::default_background: return handle_default_color(config, "background", intensity);

    case mode::palette:
        {
            const std::uint8_t index = get_palette_index();

            if (index < 8) return handle_named_palette(config, index, intensity);
            if (index < 16) return handle_named_palette(config, index - 8, intensity::bright);

            return int32_to_fcolor(fixed_palette[index]);
        }

    case mode::truecolor: return int32_to_fcolor(get_rgb());
    }

    std::unreachable();
}


auto color::handle_default_color(const config    &conf,
                                 std::string_view key,
                                 intensity intensity) noexcept -> std::expected<sdl::fcolor, error>
{
    const auto *colors = conf.get_as<toml::table>("colors");

    if (colors == nullptr) [[unlikely]]
        return error { "Table \"colors\" does not exist in configuration." }.unexpected();

    const auto *arr = colors->get(key)->as_array();

    if (arr == nullptr) [[unlikely]]
        return error { R"(Key "colors.{}" is not an array.)", key }.unexpected();

    if (auto res = read_hex_entry(*arr, std::size_t(intensity), key); res.has_value())
        return int32_to_fcolor(*res);
    else /* NOLINT */
        return res.error().unexpected();
}


auto color::handle_named_palette(const config &conf,
                                 std::uint8_t  index,
                                 intensity intensity) noexcept -> std::expected<sdl::fcolor, error>
{
    const auto *palette_table = conf.get_as<toml::table>("colors.palette");

    if (palette_table == nullptr) [[unlikely]]
        return error { "Table \"colors.palette\" does not exist in configuration." }.unexpected();

    std::string_view sub = intensity == intensity::bright ? "bright"
                         : intensity == intensity::dim    ? "dim"
                                                          : "normal";

    const auto *node = palette_table->get(sub);

    if (node == nullptr and intensity == intensity::dim)
    {
        node = palette_table->get("normal");

        const auto *normal = node->as_array();
        if (normal == nullptr) [[unlikely]]
            return error { R"(Key "colors.palette.normal" is not an array.)" }.unexpected();

        if (auto res = read_hex_entry(*normal, index, "normal"); res.has_value())
            return adjust_color_brightness(int32_to_fcolor(*res), .5F);
        else /* NOLINT */
            return res.error().unexpected();
    }

    if (node == nullptr) [[unlikely]]
        return error { R"(Key "colors.palette.{}" does not exist.)", sub }.unexpected();

    const auto *arr = node->as_array();

    if (arr == nullptr) [[unlikely]]
        return error { R"(Key "colors.palette.{}" is not an array.)", sub }.unexpected();

    if (auto res = read_hex_entry(*arr, index, sub); res.has_value())
        return int32_to_fcolor(*res);
    else /* NOLINT */
        return res.error().unexpected();
}

