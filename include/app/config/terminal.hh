#pragma once
#include "app/config/base.hh"


namespace cart::app::conf
{
    class terminal final : public base
    {
    public:
        [[nodiscard]]
        static constexpr auto get_default() noexcept -> const terminal &
        {
            static constexpr terminal t {};
            return t;
        }


        auto parse(const toml::table &terminal) noexcept -> result<> override;


        [[nodiscard]]
        constexpr auto get_rows() const noexcept -> std::size_t
        { return m_rows; }

        [[nodiscard]]
        constexpr auto get_columns() const noexcept -> std::size_t
        { return m_columns; }

    private:
        std::size_t m_rows    = 120;
        std::size_t m_columns = 80;
    };
}
