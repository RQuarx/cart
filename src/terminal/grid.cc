#include "terminal/grid.hh"

using cart::term::grid;


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

    auto s = (*this)[0, 1];
}


auto grid::operator[](std::size_t row) const noexcept -> result<const term::row *>
{ return const_cast<grid &>(*this)[row]; }


auto grid::operator[](std::size_t begin, std::size_t end) noexcept
    -> result<std::ranges::subrange<std::deque<term::row>::iterator>>
{
    if (begin >= end)
        return error { "Invalid range, `begin` ({}) >= `end` ({})", begin, end }.unexpected();
    if (end >= rows())
        return error { "Out-of-range row access. `end` ({}) >= grid::rows() ({})", end, rows() }
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
    if (new_columns == columns()) return;
    std::deque<term::row> new_rows;

    for (const auto &row : m_rows) {}

    m_rows = std::move(new_rows);
}
