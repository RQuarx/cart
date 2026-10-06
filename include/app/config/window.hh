#pragma once
#include "app/config/base.hh"


namespace cart::app::cfg
{
    class window final : public base
    {
    public:
        static constexpr auto get_default() noexcept -> const window &
        {
            static constexpr window w {};
            return w;
        }

        auto parse(const toml::table &window) noexcept -> result<> override;

        [[nodiscard]]
        constexpr auto get_title() const noexcept -> std::string_view
        { return m_title; }

        [[nodiscard]]
        constexpr auto get_padding() const noexcept -> std::pair<std::uint32_t, std::uint32_t>
        { return { m_padding.x, m_padding.y }; }

        [[nodiscard]]
        constexpr auto get_dynamic_padding() const noexcept -> bool
        { return m_dynamic_padding; }

    private:
        std::string m_title = "cart";

        struct
        {
            std::uint32_t x = 0;
            std::uint32_t y = 0;
        } m_padding;

        bool m_dynamic_padding = false;
    };
}
