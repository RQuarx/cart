#pragma once
#include <cstdint>

#include "core/theme.hh"


namespace cart::core
{
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
            bright = 0,
            normal = 1,
            dim    = 2
        };


        [[nodiscard]]
        static constexpr auto make_default_bg() noexcept -> color
        { return { pack(mode::default_background, 0) }; }

        [[nodiscard]]
        static constexpr auto make_default_fg() noexcept -> color
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
        { return mode(m_data >> mode_shift); }

        [[nodiscard]]
        constexpr auto get_palette_index() const noexcept -> std::uint8_t
        { return std::uint8_t(m_data & payload_mask); }

        [[nodiscard]]
        constexpr auto get_rgb() const noexcept -> std::uint32_t
        { return m_data & payload_mask; }

        [[nodiscard]]
        auto resolve(const theme &theme, intensity intensity) const noexcept -> std::uint32_t;


    private:
        std::uint32_t m_data;


        constexpr color(std::uint32_t data) noexcept : m_data { data } {}


        static constexpr std::uint32_t mode_shift   = 30;
        static constexpr std::uint32_t payload_mask = (1U << mode_shift) - 1;

        static constexpr auto pack(color::mode mode, std::uint32_t payload) noexcept
            -> std::uint32_t
        { return (std::uint32_t(mode) << mode_shift) | (payload & payload_mask); }
    };
}
