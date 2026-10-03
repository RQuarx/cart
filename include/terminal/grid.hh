#pragma once
#include <cstddef>
#include <deque>

#include "shared/result.hh"
#include "terminal/row.hh"


namespace cart::term
{
    struct scroll_region
    {
        std::size_t top;    /* inclusive */
        std::size_t bottom; /* one past */
    };


    struct resize_delta
    {
        std::size_t pushed; /* scrollback lines pulled onto the screen (content moved down) */
        std::size_t pulled; /* screen lines pushed into scrollback (content moved up) */
    };


    class grid
    {
    public:
        [[nodiscard]]
        static auto create(std::size_t rows,
                           std::size_t columns,
                           std::size_t scrollback_limit = 0) noexcept -> result<grid>;


        /** @brief Screen rows: 0 = top of the screen */
        [[nodiscard]]
        constexpr auto row_at(this auto &self, std::size_t i) noexcept -> auto &
        { return self.m_lines[self.screen_base() + i]; }

        /** @brief Rows as displayed: accounts for the scrolled-back viewport */
        [[nodiscard]]
        constexpr auto view_row_at(this auto &self, std::size_t i) noexcept -> auto &
        { return self.m_lines[self.screen_base() - self.m_view_offset + i]; }

        [[nodiscard]] constexpr auto columns() const noexcept -> std::size_t { return m_columns; }
        [[nodiscard]]
        constexpr auto rows_count() const noexcept -> std::size_t
        { return m_screen_rows; }

        [[nodiscard]]
        constexpr auto scrollback_size() const noexcept -> std::size_t
        { return m_lines.size() - m_screen_rows; }

        [[nodiscard]]
        constexpr auto scrollback_limit() const noexcept -> std::size_t
        { return m_scrollback_limit; }


        /** @brief Scrolling content up/down inside a region (full screen by default) */
        auto scroll_up(scroll_region region, std::size_t n = 1) noexcept -> result<void>;
        auto scroll_down(scroll_region region, std::size_t n = 1) noexcept -> result<void>;
        auto scroll_up(std::size_t n = 1) noexcept -> result<void>
        { return scroll_up({ 0, m_screen_rows }, n); }
        auto scroll_down(std::size_t n = 1) noexcept -> result<void>
        { return scroll_down({ 0, m_screen_rows }, n); }

        auto resize(std::size_t new_rows, std::size_t new_columns) noexcept -> result<resize_delta>;

#pragma region viewport

        [[nodiscard]]
        constexpr auto view_offset() const noexcept -> std::size_t
        { return m_view_offset; }

        /** @brief Towards older content */
        void scroll_view_up(std::size_t n) noexcept;

        /** @brief Towards newer content */
        void scroll_view_down(std::size_t n) noexcept;
        void reset_view() noexcept { m_view_offset = 0; }

#pragma endregion

        /**
         * @brief Ids stay valid while the line exists, even as the deque shifts.
         *        Use these for selections, search matches, shell-integration jumps.
         */
        [[nodiscard]]
        constexpr auto id_of_screen_row(std::size_t i) const noexcept -> std::uint64_t
        { return m_evicted + screen_base() + i; }


        [[nodiscard]]
        constexpr auto find_line(std::uint64_t id) noexcept -> row *
        {
            if (id < m_evicted or id - m_evicted >= m_lines.size())
                return nullptr; /* evicted or invalid */
            return &m_lines[id - m_evicted];
        }


    private:
        std::deque<row> m_lines; /* [scrollback..., screen...] */
        std::size_t     m_screen_rows;
        std::size_t     m_columns;
        std::size_t     m_scrollback_limit;
        std::size_t     m_view_offset = 0;
        std::uint64_t   m_evicted     = 0; /* lines ever popped from the front */


        grid(std::size_t rows, std::size_t columns, std::size_t scrollback_limit);

        [[nodiscard]]
        constexpr auto screen_base() const noexcept -> std::size_t
        { return m_lines.size() - m_screen_rows; }

        void scroll_into_scrollback(std::size_t n); /* may throw bad_alloc */
        void trim_scrollback() noexcept;
    };
}
