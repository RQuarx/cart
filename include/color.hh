#pragma once
#include <cstdint>

#include "config.hh"


struct SDL_FColor;

namespace cart
{
    namespace sdl { using fcolor = ::SDL_FColor; }


    class color
    {
    public:
        enum class mode : std::uint8_t
        {
            default_background,
            default_foreground,
            palette,
            truecolor,
        };

        enum class intensity : std::uint8_t
        {
            bright,
            normal,
            dim
        };


        [[nodiscard]]
        static constexpr auto make_default_fg() noexcept -> color
        { return { pack(mode::default_background, 0) }; }

        [[nodiscard]]
        static constexpr auto make_default_bg() noexcept -> color
        { return { pack(mode::default_foreground, 0) }; }

        [[nodiscard]]
        static constexpr auto make_palette(std::uint8_t index) noexcept -> color
        { return { pack(mode::palette, index) }; }

        [[nodiscard]]
        static constexpr auto
        make_truecolor(std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept -> color
        {
            auto rgb = (std::uint32_t(r) << 16) | (std::uint32_t(g) << 8) | std::uint32_t(b);

            return { pack(mode::truecolor, rgb) };
        }


        [[nodiscard]]
        constexpr auto get_mode() const noexcept -> mode
        { return mode(this->data >> mode_shift); }

        [[nodiscard]]
        constexpr auto get_palette_index() const noexcept -> std::uint8_t
        { return std::uint8_t(this->data & payload_mask); }

        [[nodiscard]]
        constexpr auto get_rgb() const noexcept -> std::uint32_t
        { return this->data & payload_mask; }

        [[nodiscard]]
        auto get_fcolor(const config &conf, intensity intensity) const noexcept
            -> std::expected<sdl::fcolor, error>;


    private:
        std::uint32_t data;


        constexpr color(std::uint32_t data) noexcept : data { data } {}


        [[nodiscard]]
        static auto
        handle_default_color(const config &conf, std::string_view key, intensity intensity) noexcept
            -> std::expected<sdl::fcolor, error>;

        [[nodiscard]]
        static auto
        handle_named_palette(const config &conf, std::uint8_t index, intensity intensity) noexcept
            -> std::expected<sdl::fcolor, error>;


        static constexpr std::uint32_t mode_shift   = 30;
        static constexpr std::uint32_t payload_mask = (1U << mode_shift) - 1;

        static constexpr auto pack(color::mode mode, std::uint32_t payload) noexcept
            -> std::uint32_t
        { return (std::uint32_t(mode) << mode_shift) | (payload | payload_mask); }
    };
}
