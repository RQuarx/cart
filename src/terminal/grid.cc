#include <algorithm>
#include <vector>

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


auto grid::scroll_up(scroll_region r, std::size_t n) noexcept -> result<>
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


auto grid::scroll_down(scroll_region r, std::size_t n) noexcept -> result<>
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


namespace
{
    [[nodiscard]]
    constexpr auto is_empty_cell(const cart::term::cell &c) noexcept -> bool
    { return c.character.is_empty(); }
}

auto grid::resize(std::size_t new_rows, std::size_t new_columns) noexcept -> result<resize_delta>
{
    if (new_rows == 0 or new_columns == 0)
        return shared::error { "Invalid new size, 0 is not allowed." }.unexpected();

    if (new_rows == m_screen_rows and new_columns == m_columns) return resize_delta {};

    if (new_columns == m_columns)
    {
        resize_delta delta {};

        const auto grow = new_rows > m_screen_rows ? new_rows - m_screen_rows : 0UZ;
        if (grow > 0)
        {
            delta.pulled = std::min(grow, scrollback_size());
            for (auto i = delta.pulled; i < grow; ++i)
                m_lines.emplace_back(m_columns);
        }
        else
            delta.pushed = m_screen_rows - new_rows;

        m_screen_rows = new_rows;
        trim_scrollback();

        for (std::size_t i = 0; i < m_screen_rows; ++i) row_at(i).damage();

        return delta;
    }

    const auto old_scrollback = scrollback_size();

    std::vector<std::vector<cell>> logical;
    std::vector<cell>              current;
    for (const auto &line : m_lines)
    {
        if (line.attribute.has(row::attribute::wrapped))
            current.insert(current.end(), line.cells.begin(), line.cells.end());
        else
        {
            std::size_t last = line.cells.size();
            while (last > 0 and is_empty_cell(line.cells[last - 1])) last--;
            current.insert(current.end(), line.cells.begin(), line.cells.begin() + last);
            logical.emplace_back(std::move(current));
            current.clear();
        }
    }
    if (!current.empty()) logical.emplace_back(std::move(current));

    std::deque<row> reflowed;
    for (const auto &line : logical)
    {
        if (line.empty())
        {
            reflowed.emplace_back(new_columns);
            continue;
        }

        row         out { new_columns };
        std::size_t pos = 0;
        std::size_t i   = 0;
        while (i < line.size())
        {
            if (line[i].character.is_spacer())
            {
                i++;
                continue;
            }

            const bool        is_wide = (i + 1 < line.size() and line[i + 1].character.is_spacer());
            const std::size_t w       = is_wide ? 2UZ : 1UZ;

            if (new_columns == 1 and is_wide)
            {
                if (pos >= 1)
                {
                    out.attribute.set(row::attribute::wrapped, true);
                    reflowed.emplace_back(std::move(out));
                    out = row { new_columns };
                    pos = 0;
                }
                out.cells[pos] = line[i];
                pos += 1;
                i += 2;
                continue;
            }

            if (pos + w > new_columns)
            {
                out.attribute.set(row::attribute::wrapped, true);
                reflowed.emplace_back(std::move(out));
                out = row { new_columns };
                pos = 0;
                continue;
            }

            out.cells[pos] = line[i];
            if (is_wide)
            {
                out.cells[pos + 1] = line[i + 1];
                pos += 2;
                i += 2;
            }
            else
            {
                pos += 1;
                i += 1;
            }
        }
        reflowed.emplace_back(std::move(out));
    }

    m_evicted += m_lines.size();
    m_lines.clear();
    while (reflowed.size() > new_rows)
    {
        m_lines.emplace_back(std::move(reflowed.front()));
        reflowed.pop_front();
    }
    for (auto &r : reflowed) m_lines.emplace_back(std::move(r));
    while (m_lines.size() < new_rows) m_lines.emplace_back(new_columns);

    m_screen_rows = new_rows;
    m_columns    = new_columns;
    m_view_offset = 0;
    trim_scrollback();

    for (std::size_t j = 0; j < m_screen_rows; ++j) row_at(j).damage();

    resize_delta delta {};
    const auto  new_scrollback = scrollback_size();
    if (new_scrollback >= old_scrollback)
        delta.pushed = new_scrollback - old_scrollback;
    else
        delta.pulled = old_scrollback - new_scrollback;

    return delta;
}


void grid::scroll_view_up(std::size_t n) noexcept
{ m_view_offset = std::min(m_view_offset + n, scrollback_size()); }

void grid::scroll_view_down(std::size_t n) noexcept { m_view_offset -= std::min(n, m_view_offset); }

