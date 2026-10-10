#include <algorithm>
#include <ranges>

#include <spdlog/spdlog.h>

#include "terminal/grid.hh"

using cart::term::grid;

namespace cart::term::_impl
{
    struct logical_line
    {
        std::vector<cell>                         cells;
        std::vector<row::extras::uri_range>       uris;
        std::vector<row::extras::underline_range> underlines;
        std::optional<row::range>                 prompt;

        void clear()
        {
            cells.clear();
            uris.clear();
            underlines.clear();
            prompt.reset();
        }

        /** Appends the first `count` cells of `row`, shifting its ranges into line coordinates. */
        void append(const row &row, std::size_t count)
        {
            const auto base = std::uint32_t(cells.size());
            cells.insert(cells.end(), row.begin(), row.begin() + count);

            auto shift = [&](auto r, auto &dst)
            {
                if (r.begin >= count) return;
                r.clamp(count);
                r.begin += base;
                r.end   += base;

                /* Merge with the previous range if they touch and are equal,
                   so one URL spanning two source rows stays one range. */
                if (not dst.empty() and dst.back().end == r.begin and dst.back() == r)
                    dst.back().end = r.end;
                else
                    dst.push_back(std::move(r));
            };

            if (auto u = row.uris())
                for (const auto &r : *u) shift(r, uris);
            if (auto u = row.underlines())
                for (const auto &r : *u) shift(r, underlines);

            if (auto p = row.prompt_row())
            {
                row::range r = *p;
                r.clamp(std::uint32_t(count));
                r.begin += base;
                r.end   += base;

                if (prompt)
                {
                    prompt->begin = std::min(prompt->begin, r.begin);
                    prompt->end   = std::max(prompt->end, r.end);
                }
                else
                    prompt = r;
            }
        }
    };
}


auto grid::columns() const noexcept -> std::size_t
{
    if (m_rows.empty()) return 0;
    return m_rows.front().columns();
}


auto grid::rows() const noexcept -> std::size_t { return m_rows.size(); }


auto grid::operator[](std::size_t row) noexcept -> result<term::row *>
{
    if (row >= rows())
        return error { "Out-of-range row access. `row` ({}) >= grid::rows() ({})", row, rows() }
            .unexpected();
    return &m_rows[row];
}


auto grid::operator[](std::size_t row) const noexcept -> result<const term::row *>
{ return const_cast<grid &>(*this)[row]; }


auto grid::operator[](std::size_t begin, std::size_t end) noexcept
    -> result<std::ranges::subrange<std::deque<term::row>::iterator>>
{
    if (begin > end)
        return error { "Invalid range, `begin` ({}) >= `end` ({})", begin, end }.unexpected();
    if (end > rows())
        return error { "Out-of-range row access. `end` ({}) > grid::rows() ({})", end, rows() }
            .unexpected();

    return std::ranges::subrange { m_rows.begin() + begin, m_rows.begin() + end };
}


auto grid::operator[](std::size_t begin, std::size_t end) const noexcept
    -> result<std::ranges::subrange<std::deque<term::row>::const_iterator>>
{ return const_cast<grid &>(*this)[begin, end]; }


void grid::pop_front() { m_rows.pop_front(); }
void grid::push_back() { push_back(row { columns() }); }


void grid::push_back(row r)
{
    m_rows.emplace_back(std::move(r));
    mf_ensure_rows_is_under_limit();
}


void grid::set_rows_limit(std::size_t n)
{
    m_rows_limit = n;
    mf_ensure_rows_is_under_limit();
}


void grid::mf_ensure_rows_is_under_limit()
{
    while (rows() > m_rows_limit) pop_front();
}


void grid::resize(std::size_t new_columns)
{
    if (new_columns == columns() or new_columns < 2) return;
    static std::deque<term::row> new_rows;
    static _impl::logical_line   line;
    line.clear();
    new_rows.clear();

    spdlog::trace("Terminal grid resized from {} columns to {} columns.", columns(), new_columns);

    for (const auto &row : m_rows)
    {
        auto count = row.columns();

        /* A wrapped row may end in padding left by a wide character that
           didn't fit. That padding isn't real content, so leave it out. */
        if (row.is_wrapped())
            while (count > 0
                   and (row.begin() + count - 1)->attribute.has(cell::attributes::unwritten))
                count--;

        line.append(row, count);

        if (row.is_wrapped()) continue;

        grid::rewrap(line, new_columns, new_rows);
        line.clear();
    }

    if (!line.cells.empty()) grid::rewrap(line, new_columns, new_rows);

    m_rows = std::move(new_rows);
    mf_ensure_rows_is_under_limit();
}


void
grid::rewrap(_impl::logical_line &line, std::size_t new_columns, std::deque<term::row> &out_rows)
{
    auto &cells = line.cells;

    std::size_t used = cells.size();
    while (used > 0 and cells[used - 1].attribute.has(cell::attributes::unwritten)) used--;

    std::size_t pos = 0;
    do
    {
        term::row   out { new_columns };
        std::size_t limit = std::min(pos + new_columns, used);
        std::size_t end   = limit;

        if (limit < used)
        {
            while (end > pos and cells[end].content.kind() == cell::character::kind::spacer) end--;
            out.set_wrapped();
        }

        std::ranges::copy(cells.begin() + pos, cells.begin() + end, out.begin());
        for (std::size_t i = end - pos; i < limit - pos; i++)
            (out.begin() + i)->attribute.set(cell::attributes::unwritten, true);

        /* Row-level extras */
        const std::uint32_t p = pos;
        const std::uint32_t e = end;

        clip(line.uris, p, e,
             [&](auto r)
             {
                 if (!out.add_uri(std::move(r))) spdlog::warn("resize: dropped a URI range");
             });

        clip(line.underlines, p, e,
             [&](auto r)
             {
                 if (!out.add_underline(std::move(r)))
                     spdlog::warn("resize: dropped an underline range");
             });

        if (line.prompt and line.prompt->end > p and line.prompt->begin < e)
        {
            term::row::range r { std::max(line.prompt->begin, p) - p,
                                 std::min(line.prompt->end, e) - p };
            out.set_prompt_row(r);
        }

        /* Cells moved, so whatever was drawn before is stale. */
        out.set_dirty();

        out_rows.emplace_back(std::move(out));
        pos = end;
    } while (pos < used);
}
