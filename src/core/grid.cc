#include "core/grid.hh"

using cart::core::grid;


auto grid::create(std::size_t rows, std::size_t columns) noexcept -> result<grid>
{ return grid { rows, columns }; }


grid::grid(std::size_t rows, std::size_t columns) : m_rows { rows }, m_columns { columns } {}


auto grid::scroll_up(std::size_t n) noexcept -> result<void>
{
    for (auto i = 0UZ; i < n; i++)
    {
        m_scrollback.emplace_back(std::move(m_rows.front()));
        m_rows.pop_front();
        if (m_scrollback.size() > m_scrollback_limit) m_scrollback.pop_front();
        m_rows.emplace_back(m_columns);
    }

    return {};
}


auto grid::scroll_down(std::size_t n) noexcept -> result<void>
{
    for (std::size_t i = 0; i < n and !m_scrollback.empty(); i++)
    {
        m_rows.emplace_front(std::move(m_scrollback.back()));
        m_scrollback.pop_back();
        m_rows.pop_back();
    }

    return {};
}


auto grid::resize(std::size_t new_rows, std::size_t new_columns) noexcept -> result<void>
{
    /* TODO */
    return {};
}


