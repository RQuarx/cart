#pragma region "row::extras impl"
#include <algorithm>

#include "terminal/row.hh"

namespace
{
    /**
     * @brief Remove a range from a sorted, non-overlapping range list.
     * @note A range that strictly contains the hole is split into two.
     */
    template <typename T>
    void erase_in(std::vector<T> &v, cart::term::row::range range)
    {
        for (std::size_t i = 0; i < v.size();)
        {
            T &r = v[i];

            if (r.begin >= range.end) break; /* sorted: nothing further can overlap */
            if (r.end <= range.begin)        /* entirely left of the hole */
            {
                i++;
                continue;
            }

            if (r.begin < range.begin and r.end > range.end) /* hole is strictly inside: split */
            {
                T tail     = r;
                tail.begin = range.end;
                r.end      = range.begin;
                v.insert(v.begin() + static_cast<std::ptrdiff_t>(i) + 1, std::move(tail));
                break; /* r is invalidated; nothing further overlaps */
            }

            if (r.begin < range.begin) /* clip the right side */
            {
                r.end = range.begin;
                i++;
            }
            else if (r.end > range.end) /* clip the left side */
            {
                r.begin = range.end;
                i++;
            }
            else
                v.erase(v.begin() + static_cast<std::ptrdiff_t>(i)); /* fully covered */
        }
    }

    /**
     * @brief Insert `value`, replacing anything under it and merging with the
     *        previous/next range when they touch and have the same payload.
     */
    template <typename T>
    void put_in(std::vector<T> &v, T value)
    {
        erase_in(v, { value.begin, value.end });

        const auto pos
            = std::lower_bound(v.begin(), v.end(), value.begin,
                               [](const T &r, std::uint32_t col) { return r.begin < col; });
        const auto idx = std::size_t(pos - v.begin());

        bool merged = false;

        if (idx > 0 and v[idx - 1].end == value.begin and v[idx - 1] == value)
        {
            v[idx - 1].end = value.end;
            merged         = true;
        }

        if (idx < v.size() and v[idx].begin == value.end and v[idx] == value)
        {
            if (merged)
            {
                v[idx - 1].end = v[idx].end;
                v.erase(v.begin() + static_cast<std::ptrdiff_t>(idx));
            }
            else
            {
                v[idx].begin = value.begin;
                merged       = true;
            }
        }

        if (!merged) v.insert(v.begin() + static_cast<std::ptrdiff_t>(idx), std::move(value));
    }
}


using extras = cart::term::row::extras;


auto extras::mf_uris() noexcept -> std::optional<std::vector<uri_range> *>
{
    if (m_data == nullptr) return std::nullopt;
    return &m_data->first;
}


auto extras::mf_underlines() noexcept -> std::optional<std::vector<underline_range> *>
{
    if (m_data == nullptr) return std::nullopt;
    return &m_data->second;
}


auto extras::uris() const noexcept -> std::optional<std::span<const uri_range>>
{
    return const_cast<extras &>(*this).mf_uris().transform([](const std::vector<uri_range> *uris)
                                                           { return std::span { *uris }; });
}


auto extras::underlines() const noexcept -> std::optional<std::span<const underline_range>>
{
    return const_cast<extras &>(*this).mf_underlines().transform(
        [](const std::vector<underline_range> *underlines) { return std::span { *underlines }; });
}


void extras::clear()
{
    if (auto res = mf_uris()) (*res)->clear();
    if (auto res = mf_underlines()) (*res)->clear();
}


void extras::add_uri(uri_range range)
{
    auto *uris = *mf_uris().or_else(
        [&]
        {
            m_data = std::make_unique<range_pair>();
            return mf_uris();
        });

    put_in(*uris, std::move(range));
}


void extras::add_underline(underline_range range)
{
    auto *underlines = *mf_underlines().or_else(
        [&]
        {
            m_data = std::make_unique<range_pair>();
            return mf_underlines();
        });

    put_in(*underlines, range);
}


void extras::erase_uri(range range)
{
    auto _ = mf_uris().and_then(
        [&](std::vector<uri_range> *r)
        {
            erase_in(*r, range);
            return mf_uris();
        });
}


void extras::erase_underline(range range)
{
    auto _ = mf_underlines().and_then(
        [&](std::vector<underline_range> *r)
        {
            erase_in(*r, range);
            return mf_underlines();
        });
}

#pragma endregion

#pragma region "row implementation"
#include <spdlog/spdlog.h>

using cart::term::row;


row::row(std::size_t columns) : m_columns { columns } {}


auto row::operator[](std::size_t col) noexcept -> result<cell *>
{
    if (col >= m_columns.size())
        return error { "Attempted to access out-of-range column {} (row::columns() = {})", col,
                       columns() }
            .unexpected();
    return &m_columns[col];
}

auto row::operator[](std::size_t col) const noexcept -> result<const cell *>
{
    return const_cast<row &>(*this)[col].transform([](cell *c) -> const cell * { return c; });
}


auto row::operator[](range r) noexcept -> result<std::span<cell>>
{
    if (r.begin >= columns())
        return error { "Invalid range, begin ({}) >= row::columns() ({})", r.begin, columns() }
            .unexpected();
    if (r.begin >= r.end)
        return error { "Invalid range, begin ({}) >= end ({})", r.begin, r.end }.unexpected();
    return std::span { m_columns.begin() + r.begin, m_columns.begin() + r.clamp(columns()).end };
}

auto row::operator[](range r) const noexcept -> result<std::span<const cell>>
{
    return const_cast<row &>(*this)[r].transform([](std::span<cell> c) -> std::span<const cell>
                                                 { return c; });
}


auto row::columns() const noexcept -> std::size_t { return m_columns.size(); }

auto row::is_wrapped() const noexcept -> bool { return m_attribute.has(attributes::wrapped); }
auto row::is_dirty() const noexcept -> bool { return !m_attribute.has(attributes::clean); }
auto row::prompt_row() const noexcept -> std::optional<row::range>
{
    if (m_attribute.has(attributes::prompt_row)) return m_attribute.prompt_range;
    return std::nullopt;
}

void row::set_wrapped(bool state) noexcept { m_attribute.set(attributes::wrapped, state); }
void row::set_dirty(bool state) noexcept { m_attribute.set(attributes::clean, !state); }
void row::set_prompt_row(std::optional<row::range> prompt_range) noexcept
{
    m_attribute.set(attributes::prompt_row, prompt_range.has_value());
    m_attribute.prompt_range = prompt_range.value_or(range {});
}


auto row::uris() const noexcept -> std::optional<std::span<const extras::uri_range>>
{ return m_extras.uris(); }


auto row::underlines() const noexcept -> std::optional<std::span<const extras::underline_range>>
{ return m_extras.underlines(); }

auto row::add_uri(extras::uri_range range) noexcept -> result<>
{
    if (range.begin >= columns())
        return error { "Invalid range, begin ({}) >= row::columns() ({})", range.begin, columns() }
            .unexpected();
    if (range.begin >= range.end)
        return error { "Invalid range, begin ({}) >= end ({})", range.begin, range.end }
            .unexpected();
    m_extras.add_uri(range);
    return {};
}

auto row::add_underline(extras::underline_range range) noexcept -> result<>
{
    if (range.begin >= columns())
        return error { "Invalid range, begin ({}) >= row::columns() ({})", range.begin, columns() }
            .unexpected();
    if (range.begin >= range.end)
        return error { "Invalid range, begin ({}) >= end ({})", range.begin, range.end }
            .unexpected();
    m_extras.add_underline(range);
    return {};
}

void row::erase_uri(row::range range)
{
    if (range.begin >= columns())
    {
        spdlog::error("Invalid range, begin ({}) >= row::columns() ({})", range.begin, columns());
        return;
    }

    if (range.begin >= range.end)
    {
        spdlog::warn("Invalid range, begin ({}) >= end ({})", range.begin, range.end);
        return;
    }

    m_extras.erase_uri(range);
}

void row::erase_underline(row::range range)
{
    if (range.begin >= columns())
    {
        spdlog::error("Invalid range, begin ({}) >= row::columns() ({})", range.begin, columns());
        return;
    }

    if (range.begin >= range.end)
    {
        spdlog::warn("Invalid range, begin ({}) >= end ({})", range.begin, range.end);
        return;
    }

    m_extras.erase_underline(range);
}
