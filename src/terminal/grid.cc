#include "terminal/grid.hh"

using cart::term::grid;


auto grid::create(std::size_t rows, std::size_t columns, std::size_t scrollback_limit) noexcept
    -> result<grid>

{
    if (rows == 0 or columns == 0)
        return shared::error { "Invalid size passed (rows: {}, columns: {})", rows, columns }
            .unexpected();
    return grid { rows, columns, scrollback_limit };
}


grid::grid(std::size_t rows, std::size_t columns, std::size_t scrollback_limit)
    : m_screen_rows { rows }, m_columns { columns }, m_scrollback_limit { scrollback_limit }
{
    for (std::size_t i = 0; i < rows; i++) m_lines.emplace_back(columns);
}


void grid::trim_scrollback() noexcept
{
    while (scrollback_size() > m_scrollback_limit)
    {
        m_lines.pop_front();
        m_evicted++;
    }

    m_view_offset = std::min(m_view_offset, scrollback_size());
}


void grid::scroll_into_scrollback(std::size_t n)
{
    for (std::size_t i = 0; i < n; i++)
    {
        if (scrollback_size() >= m_scrollback_limit)
        {
            /* full: recycle the oldest line as the new bottom row */
            row recycled = std::move(m_lines.front());
            m_lines.pop_front();
            m_evicted++;

            recycled.erase();
            m_lines.push_back(std::move(recycled));
        }
        else
            m_lines.emplace_back(m_columns);

        /* Keep the user's scrolled-back view anchored to the same text */
        if (m_view_offset > 0) m_view_offset = std::min(m_view_offset + 1, scrollback_size());
    }
}


auto grid::scroll_up(scroll_region r, std::size_t n) noexcept -> result<void>
{
    n = std::min(n, r.bottom - r.top);
    if (n == 0) return {};

    const bool full_screen = r.top == 0 and r.bottom == m_screen_rows;

    if (full_screen and m_scrollback_limit > 0)
    {
        scroll_into_scrollback(n);
        return {};
    }

    /* partial region (or no scrollback): rotate the top n rows to the bottom and blank them */
    const auto base  = m_lines.begin() + static_cast<std::ptrdiff_t>(screen_base());
    const auto first = base + static_cast<std::ptrdiff_t>(r.top);
    const auto last  = base + static_cast<std::ptrdiff_t>(r.bottom);

    std::rotate(first, first + static_cast<std::ptrdiff_t>(n), last);

    for (std::size_t i = r.bottom - n; i < r.bottom; i++) row_at(i).erase();
    for (std::size_t i = r.top; i < r.bottom; i++)
        row_at(i).damage(); /* no pixel-shift optimization yet: repaint the region */

    return {};
}


auto grid::scroll_down(scroll_region r, std::size_t n) noexcept -> result<void>
{
    n = std::min(n, r.bottom - r.top);
    if (n == 0) return {};

    const auto base  = m_lines.begin() + static_cast<std::ptrdiff_t>(screen_base());
    const auto first = base + static_cast<std::ptrdiff_t>(r.top);
    const auto last  = base + static_cast<std::ptrdiff_t>(r.bottom);

    /* bottom n rows rotate to the top, they are blanked. Scrollback is never touched. */
    std::rotate(first, last - static_cast<std::ptrdiff_t>(n), last);

    for (std::size_t i = r.top; i < r.top + n; ++i) row_at(i).erase();

    for (std::size_t i = r.top; i < r.bottom; ++i) row_at(i).damage();

    return {};
}


auto grid::resize(std::size_t new_rows, std::size_t new_columns) noexcept -> result<resize_delta>
{
    if (new_rows == 0 or new_columns == 0)
        return shared::error { "Invalid new size, 0 is not allowed." }.unexpected();

    resize_delta delta {};

    if (new_columns != m_columns)
    {
        for (auto &line : m_lines) /* no reflow yet: truncate / pad */
            line.resize(new_columns);
        m_columns = new_columns;
    }

    if (new_rows > m_screen_rows)
    {
        const auto grow = new_rows - m_screen_rows;
        delta.pulled
            = std::min(grow, scrollback_size()); /* computed before m_screen_rows changes */

        for (auto i = delta.pulled; i < grow; ++i)
            m_lines.emplace_back(m_columns); /* not enough history: blank rows at the bottom */
    }
    else
        delta.pushed = m_screen_rows - new_rows;

    m_screen_rows = new_rows;
    trim_scrollback();

    for (std::size_t i = 0; i < m_screen_rows; ++i) row_at(i).damage();

    return delta;
}


void grid::scroll_view_up(std::size_t n) noexcept
{ m_view_offset = std::min(m_view_offset + n, scrollback_size()); }

void grid::scroll_view_down(std::size_t n) noexcept { m_view_offset -= std::min(n, m_view_offset); }

