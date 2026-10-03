#include <iostream>
#include <string>

#include "terminal/grid.hh"

using cart::term::cell;
using cart::term::grid;
using cart::term::row;

static void set_text(grid &g, std::size_t r, std::string_view s)
{
    auto &&line = g.row_at(r);
    for (std::size_t i = 0; i < s.size() && i < line.cells.size(); i++)
        line.cells[i].character = cell::character::make_codepoint(s[i]);
}

static auto get_text(grid &g, std::size_t r, std::size_t cols) -> std::string
{
    auto &&line = g.row_at(r);
    std::string out;
    for (std::size_t i = 0; i < cols && i < line.cells.size(); i++)
    {
        auto &ch = line.cells[i].character;
        if (ch.is_spacer())
            out += "<W>";
        else if (ch.is_empty())
            out += ".";
        else
            out += (char)ch.get_codepoint();
    }
    return out;
}

static int failures = 0;
#define CHECK(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cout << "FAIL: " << msg << " (line " << __LINE__ << ")\n"; \
            failures++; \
        } else { \
            std::cout << "ok: " << msg << "\n"; \
        } \
    } while (0)

int main()
{
    {
        auto res = grid::create(24, 80);
        CHECK(res.has_value(), "create 24x80");
        CHECK(res->rows_count() == 24, "rows_count == 24");
        CHECK(res->columns() == 80, "columns == 80");
        CHECK(res->row_at(0).cells.size() == 80, "row width 80");
    }

    {
        auto res = grid::create(1, 10, 10);
        grid &g  = *res;
        set_text(g, 0, "ABCDEFGHIJ");
        auto r = g.resize(1, 5);
        CHECK(r.has_value(), "resize 1x10 -> 1x5 ok");
        CHECK(g.columns() == 5, "columns now 5");
        CHECK(g.rows_count() == 1, "visible rows still 1");
        CHECK(g.scrollback_size() == 1, "1 row pushed to scrollback");
        CHECK(r->pushed == 1, "delta pushed 1");
        std::string vis = get_text(g, 0, 5);
        CHECK(vis == "FGHIJ", std::string("visible is tail 'FGHIJ', got '") + vis + "'");
        auto &&sb = g.scrollback_at(0);
        std::string sbt;
        for (std::size_t i = 0; i < 5; i++)
            sbt += (char)sb.cells[i].character.get_codepoint();
        CHECK(sbt == "ABCDE", std::string("scrollback is head 'ABCDE', got '") + sbt + "'");
        CHECK(sb.attribute.has(row::attribute::wrapped), "scrollback head marked wrapped");
    }

    {
        auto res = grid::create(1, 6, 10);
        grid &g  = *res;
        auto &&line              = g.row_at(0);
        line.cells[0].character  = cell::character::make_codepoint('a');
        line.cells[1].character  = cell::character::make_codepoint('b');
        line.cells[2].character  = cell::character::make_codepoint('c');
        line.cells[3].character  = cell::character::make_codepoint('d');
        line.cells[4].character  = cell::character::make_codepoint(0x4E2D);
        line.cells[5].character  = cell::character::make_spacer();
        auto r                   = g.resize(2, 5);
        CHECK(r.has_value(), "wide resize ok");
        CHECK(g.rows_count() == 2, "wide: 2 visible rows");
        std::string r0 = get_text(g, 0, 5);
        CHECK(r0 == "abcd.", std::string("wide: row0 'abcd.', got '") + r0 + "'");
        CHECK(g.row_at(0).attribute.has(row::attribute::wrapped), "wide: row0 wrapped");
        CHECK(g.row_at(1).cells[0].character.get_codepoint() == 0x4E2D, "wide: leading kept");
        CHECK(g.row_at(1).cells[1].character.is_spacer(), "wide: spacer kept together");
    }

    {
        auto res = grid::create(2, 5);
        grid &g  = *res;
        set_text(g, 0, "hello");
        set_text(g, 1, "world");
        auto r = g.resize(4, 5);
        CHECK(r.has_value(), "grow 2->4 ok");
        CHECK(g.rows_count() == 4, "grow rows_count 4");
        CHECK(get_text(g, 0, 5) == "hello", "grow row0 kept");
        CHECK(get_text(g, 1, 5) == "world", "grow row1 kept");
        CHECK(get_text(g, 2, 5) == ".....", "grow row2 blank");
        CHECK(get_text(g, 3, 5) == ".....", "grow row3 blank");
    }

    {
        auto res = grid::create(3, 4);
        grid &g  = *res;
        set_text(g, 1, "ab");
        auto r = g.resize(3, 4);
        CHECK(r.has_value(), "same-dims ok");
        CHECK(r->pushed == 0 and r->pulled == 0, "same-dims delta zero");
        CHECK(get_text(g, 1, 4) == "ab..", "same-dims preserved");
    }

    {
        auto res = grid::create(2, 2);
        grid &g  = *res;
        CHECK(!g.resize(0, 5).has_value(), "0 rows rejected");
        CHECK(!g.resize(5, 0).has_value(), "0 cols rejected");
    }

    {
        auto res = grid::create(2, 4, 10);
        grid &g  = *res;
        set_text(g, 0, "abcd");
        set_text(g, 1, "efgh");
        g.row_at(0).attribute.set(row::attribute::wrapped, true);
        auto r = g.resize(2, 8);
        CHECK(r.has_value(), "join resize ok");
        CHECK(g.rows_count() == 2, "join rows 2");
        CHECK(get_text(g, 0, 8) == "abcdefgh", "join row0 is full logical");
        CHECK(get_text(g, 1, 8) == "........", "join row1 blank");
        CHECK(!g.row_at(0).attribute.has(row::attribute::wrapped), "join single row unwrapped");
    }

    {
        auto res = grid::create(1, 12, 1);
        grid &g  = *res;
        set_text(g, 0, "ABCDEFGHIJKL");
        auto r = g.resize(1, 4);
        CHECK(r.has_value(), "big shrink ok");
        CHECK(g.rows_count() == 1, "shrink visible 1");
        CHECK(g.scrollback_size() == 1, "scrollback capped at limit 1");
        CHECK(get_text(g, 0, 4) == "IJKL", "shrink visible is tail");
        auto &&sb = g.scrollback_at(0);
        std::string sbt;
        for (std::size_t i = 0; i < 4; i++)
            sbt += (char)sb.cells[i].character.get_codepoint();
        CHECK(sbt == "EFGH", std::string("scrollback keeps newest overflow, got '") + sbt + "'");
    }

    {
        auto res = grid::create(1, 3, 10);
        grid &g  = *res;
        auto &&line              = g.row_at(0);
        line.cells[0].character  = cell::character::make_codepoint('x');
        line.cells[1].character  = cell::character::make_codepoint(0x4E2D);
        line.cells[2].character  = cell::character::make_spacer();
        auto r                   = g.resize(1, 1);
        CHECK(r.has_value(), "width-1 resize ok");
        CHECK(g.columns() == 1, "width-1 columns");
        CHECK(g.scrollback_size() == 1, "width-1 overflow to scrollback");
        CHECK(!g.row_at(0).cells[0].character.is_spacer(), "width-1 visible not a spacer");
        CHECK(!g.scrollback_at(0).cells[0].character.is_spacer(), "width-1 scrollback not a spacer");
    }

    {
        auto res = grid::create(1, 4, 10);
        grid &g  = *res;
        set_text(g, 0, "ABCD");
        CHECK(g.resize(1, 2).has_value(), "shrink for pullback");
        CHECK(g.scrollback_size() == 1, "pullback setup scrollback 1");
        auto r = g.resize(2, 2);
        CHECK(r.has_value(), "grow pulls back");
        CHECK(r->pulled == 1, "pullback delta pulled 1");
        CHECK(g.rows_count() == 2, "pullback visible 2");
        CHECK(g.scrollback_size() == 0, "pullback scrollback drained");
        CHECK(get_text(g, 0, 2) == "AB", "pullback row0 restored");
        CHECK(get_text(g, 1, 2) == "CD", "pullback row1 restored");
    }

    {
        auto res = grid::create(2, 4, 10);
        grid &g  = *res;
        set_text(g, 0, "abcd");
        set_text(g, 1, "efgh");
        CHECK(g.scroll_up(1).has_value(), "interplay scroll_up");
        auto r = g.resize(2, 2);
        CHECK(r.has_value(), "interplay resize ok");
        CHECK(g.rows_count() == 2, "interplay visible 2");
        CHECK(get_text(g, 0, 2) == "gh", "interplay visible row0");
        CHECK(get_text(g, 1, 2) == "..", "interplay visible row1 blank");
        CHECK(g.scrollback_size() == 3, "interplay scrollback 3");
        CHECK(r->pushed == 2, "interplay delta pushed 2");
    }

    if (failures == 0)
        std::cout << "\nALL TESTS PASSED\n";
    else
        std::cout << "\n" << failures << " FAILURES\n";
    return failures == 0 ? 0 : 1;
}
