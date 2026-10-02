#pragma once
#include <cstddef>
#include <deque>

#include "core/row.hh"
#include "shared/result.hh"


namespace cart::core
{
    class grid
    {

    public:
        [[nodiscard]]
        static auto create(std::size_t rows, std::size_t columns) noexcept -> result<grid>;

        [[nodiscard]]
        auto row_at(this auto &&self, std::size_t i) noexcept -> row &&
        { return decltype(self)(self).m_rows[i]; }

        [[nodiscard]]
        constexpr auto rows_count() const noexcept -> std::size_t { return m_rows.size(); }

        [[nodiscard]]
        constexpr auto columns() const noexcept -> std::size_t { return m_columns; }


        auto scroll_up(std::size_t n = 1) noexcept -> result<void>;
        auto scroll_down(std::size_t n = 1) noexcept -> result<void>;
        auto resize(std::size_t new_rows, std::size_t new_columns) noexcept -> result<void>;


    private:
        std::deque<row> m_rows;
        std::deque<row> m_scrollback;
        std::size_t     m_columns;
        std::size_t     m_scrollback_limit = 0;


        grid(std::size_t rows, std::size_t columns);
    };
}
