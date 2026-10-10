#include <cstdint>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

template <typename T1, typename T2>
auto operator<<(std::ostream &os, const std::pair<T1, T2> &p) -> std::ostream &
{ return os << "(" << p.first << ", " << p.second << ")"; }


#include "terminal/grid.hh"

namespace cart::term
{
    inline auto operator<<(std::ostream &os, const row::range &r) -> std::ostream &
    {
        if (r.begin == row::range::unset and r.end == row::range::unset)
            return os << "range{unset}";
        return os << "range[" << r.begin << ", " << r.end << ")";
    }
}

#include <boost/ut.hpp>

using namespace boost::ut;
using cart::term::cell;
using cart::term::color;
using cart::term::grid;
using cart::term::row;
using cart::term::underline_style;
using character = cart::term::cell::character;


namespace
{
    using bounds_t = std::vector<std::pair<std::uint32_t, std::uint32_t>>;
    using strs     = std::vector<std::string>;

    template <typename Opt>
    [[maybe_unused]]
    auto bounds(const Opt &opt) -> bounds_t
    {
        bounds_t out;
        if (!opt) return out;
        for (const auto &r : *opt) out.emplace_back(r.begin, r.end);
        return out;
    }

    [[maybe_unused]]
    auto uri(std::uint32_t b, std::uint32_t e, std::string u = "http://a", std::size_t id = 1)
        -> row::extras::uri_range
    {
        row::extras::uri_range r {};
        r.begin = b;
        r.end   = e;
        r.uri   = std::move(u);
        r.id    = id;
        return r;
    }

    [[maybe_unused]]
    auto underline(std::uint32_t b, std::uint32_t e, int style = 1) -> row::extras::underline_range
    {
        row::extras::underline_range r {
            { b, e },
            color::make_default_fg(), static_cast<underline_style>(style)
        };
        return r;
    }

    [[maybe_unused]] auto cell_at(row &r, std::size_t c) -> cell & { return **r[c]; }
    [[maybe_unused]] auto row_at(grid &g, std::size_t i) -> row & { return **g[i]; }


    /** Writes `ch` into a cell the way a real write would: content set, `blank` cleared. */
    [[maybe_unused]]
    void put(row &r, std::size_t col, character ch)
    {
        auto &c   = cell_at(r, col);
        c.content = ch;
        c.attribute.set(cell::attributes::unwritten, false);
    }

    /**
     * Builds a row holding ASCII `text`. Cells holding text are written (not blank); every cell
     * after the text keeps its default, blank state. A wrapped row is expected to be filled
     * entirely.
     */
    [[maybe_unused]]
    auto make_row(std::size_t cols, std::string_view text, bool wrapped = false) -> row
    {
        row         r { cols };
        std::size_t i = 0;
        for (char ch : text) put(r, i++, character::codepoint(char32_t(ch)));
        if (wrapped) r.set_wrapped();
        return r;
    }

    /** Row text with trailing spaces / NULs trimmed (spacers emit nothing). */
    [[maybe_unused]]
    auto text(const row &r) -> std::string
    {
        auto s = r.to_string([](std::uint32_t) -> std::string_view { return ""; });
        while (!s.empty() and (s.back() == ' ' or s.back() == '\0')) s.pop_back();
        return s;
    }


    [[maybe_unused]]
    auto texts(grid &g) -> strs
    {
        strs out;
        for (std::size_t i = 0; i < g.rows(); i++) out.push_back(text(row_at(g, i)));
        return out;
    }


    [[maybe_unused]]
    auto wrapped_flags(grid &g) -> std::vector<bool>
    {
        std::vector<bool> out;
        out.reserve(g.rows());
        for (std::size_t i = 0; i < g.rows(); i++) out.push_back(row_at(g, i).is_wrapped());
        return out;
    }


    template <typename... R>
    [[maybe_unused]]
    auto make_grid(R &&...rows) -> grid
    {
        grid g;
        (g.push_back(std::forward<R>(rows)), ...);
        return g;
    }

}
