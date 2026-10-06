#pragma once
#include <array>
#include <cstdint>

#include "app/config/base.hh"


namespace cart::app::cfg
{
    class colors final : public base
    {
    public:
        enum class intensity : std::uint8_t
        {
            bright = 0,
            normal = 1,
            dim    = 2
        };

        static constexpr auto unset = std::numeric_limits<std::uint32_t>::max();

    private:
        template <typename T>
        struct triple
        {
            std::array<T, 3> values;


            [[nodiscard]]
            constexpr static auto all(T all) noexcept -> triple
            { return { all, all, all }; }

            [[nodiscard]]
            constexpr auto operator[](this auto &&self, intensity i) noexcept -> auto &
            { return self.values[std::to_underlying(i)]; }
        };


        [[nodiscard]]
        static constexpr auto intensity_to_string(intensity intense) noexcept -> std::string_view
        {
            switch (intense)
            {
            case intensity::bright: return "bright";
            case intensity::normal: return "normal";
            case intensity::dim:    return "dim";
            }
        }

    public:
        static auto get_default() noexcept -> const colors &;
        auto        parse(const toml::table &colors) noexcept -> result<> override;


        [[nodiscard]]
        constexpr auto get_palette(intensity intensity) const noexcept
            -> std::span<const std::uint32_t, 8>
        { return m_palette[intensity]; }


        [[nodiscard]]
        constexpr auto get_foreground(intensity intensity) const noexcept -> std::uint32_t
        { return m_foreground[intensity]; }

        [[nodiscard]]
        constexpr auto get_background(intensity intensity) const noexcept -> std::uint32_t
        { return m_background[intensity]; }

        [[nodiscard]]
        constexpr auto get_bold_is_bright() const noexcept -> bool
        { return m_bold_as_bright; }


    private:
        triple<std::array<std::uint32_t, 8>> m_palette { { {
            { 0x404040, 0xFF0000, 0x00FF00, 0xFFFF00, 0x0000FF, 0xFF00FF, 0x00FFFF, 0xFFFFFF },
            { 0x000000, 0xCD0000, 0x00CD00, 0xCDCD00, 0x0000CD, 0xCD00CD, 0x00CDCD, 0xFAEBD7 },
            { 0x000000, 0x872B22, 0x549E4E, 0xAAAB34, 0x0F2353, 0x972596, 0x31A7A6, 0xE3C7A1 },
        } } };

        triple<std::uint32_t> m_foreground { 0xFFFFFF, 0xFFFFFF, 0xE3C7A1 };
        triple<std::uint32_t> m_background { 0x404040, 0x000000, 0x000000 };
        bool                  m_bold_as_bright = true;


        [[nodiscard]]
        constexpr static auto make_dim(std::uint32_t color) noexcept -> std::uint32_t
        { return (color >> 1) & 0x7F7F7F; }


        void mf_parse_foreground_and_background(std::string_view key, const toml::node &node);
        void mf_parse_palette(std::string_view key, const toml::node &node);
    };
}
