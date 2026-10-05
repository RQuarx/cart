#pragma once
#include "app/config/base.hh"


namespace cart::app::conf
{
    class scrolling final : public base
    {
    public:
        [[nodiscard]]
        static constexpr auto get_default() noexcept -> const scrolling &
        {
            static constexpr scrolling s {};
            return s;
        }


        auto parse(const toml::table &scrolling) noexcept -> result<> override;


        [[nodiscard]]
        constexpr auto get_scrollback() const noexcept -> std::size_t
        { return m_scrollback; }

        [[nodiscard]]
        constexpr auto get_multiplier() const noexcept -> std::uint8_t
        { return m_multiplier; }

    private:
        std::size_t  m_scrollback = 100'000;
        std::uint8_t m_multiplier = 3;
    };
}
