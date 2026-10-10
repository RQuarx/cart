#include <utils.cc> /* NOLINT */


suite<"grid storage"> storage_suite = []
{
    "default grid is empty"_test = []
    {
        grid g;
        expect(that % g.rows() == 0UZ);
        expect(that % g.columns() == 0UZ);
    };

    "push_back(row) appends and defines columns()"_test = []
    {
        grid g;
        g.push_back(row { 4 });
        g.push_back(row { 4 });
        expect(that % g.rows() == 2UZ);
        expect(that % g.columns() == 4UZ);
    };

    "push_back() appends a unwritten row of the same width"_test = []
    {
        auto g = make_grid(make_row(6, "x"));
        g.push_back();
        expect(that % g.rows() == 2UZ);
        expect(that % row_at(g, 1).columns() == 6UZ);
        expect(text(row_at(g, 1)).empty());
    };

    "operator[] returns the stored row, not a copy"_test = []
    {
        auto g = make_grid(make_row(4, "a"), make_row(4, "b"));

        put(row_at(g, 1), 0, character::codepoint(U'z'));

        expect(text(row_at(g, 1)) == "z");
        expect(text(row_at(g, 0)) == "a");
    };

    "operator[] on const grid"_test = []
    {
        const auto g = make_grid(make_row(4, "a"), make_row(4, "b"));
        expect(g[0].has_value());
        expect(g[1].has_value());
    };

    "operator[] past the end is an error"_test = []
    {
        auto g = make_grid(make_row(4, "a"), make_row(4, "b"));
        expect(!g[3].has_value());
        expect(!g[100].has_value());
    };

    /* The check is `row > rows()`, so row == rows() slips through and hands back a pointer one
     * past the last row. */
    "operator[] exactly one past the end is an error"_test = []
    {
        auto g = make_grid(make_row(4, "a"), make_row(4, "b"));
        expect(!g[2].has_value()) << "row == rows() must be rejected";

        grid empty;
        expect(!empty[0].has_value()) << "indexing an empty grid must be rejected";
    };

    "pop_front drops the oldest row"_test = []
    {
        auto g = make_grid(make_row(4, "a"), make_row(4, "b"), make_row(4, "c"));
        g.pop_front();
        expect(texts(g) == strs { "b", "c" });
    };
};


suite<"grid row ranges"> range_suite = []
{
    auto make = [] { return make_grid(make_row(4, "a"), make_row(4, "b"), make_row(4, "c")); };

    "full range"_test = [&]
    {
        auto g = make();
        auto s = g[0UZ, 3UZ];
        expect(s.has_value());
        expect(that % std::ranges::size(*s) == 3UZ);
    };

    "middle range"_test = [&]
    {
        auto g = make();
        auto s = g[1UZ, 3UZ];
        expect(s.has_value());
        expect(that % std::ranges::size(*s) == 2UZ);
        expect(text(*s->begin()) == "b");
    };

    "range refers to the stored rows"_test = [&]
    {
        auto g = make();
        auto s = g[1UZ, 2UZ];
        expect(s.has_value());
        put(*s->begin(), 0, character::codepoint(U'Q'));
        expect(text(row_at(g, 1)) == "Q");
    };

    "empty range is valid"_test = [&]
    {
        auto g = make();
        auto s = g[1UZ, 1UZ];
        expect(s.has_value());
        expect(that % std::ranges::size(*s) == 0UZ);
    };

    "begin > end is an error"_test = [&]
    {
        auto g = make();
        expect(!g[2UZ, 1UZ].has_value());
    };

    "end > rows is an error"_test = [&]
    {
        auto g = make();
        expect(!g[0UZ, 4UZ].has_value());
        expect(!g[5UZ, 6UZ].has_value());
    };

    "range on const grid"_test = [&]
    {
        const auto g = make();
        auto       s = g[0UZ, 2UZ];
        expect(s.has_value());
        expect(that % std::ranges::size(*s) == 2UZ);
        expect(!g[0UZ, 9UZ].has_value());
    };
};


suite<"grid row limit"> limit_suite = []
{
    "push_back evicts the oldest rows past the limit"_test = []
    {
        grid g;
        g.set_rows_limit(2);
        g.push_back(make_row(4, "a"));
        g.push_back(make_row(4, "b"));
        g.push_back(make_row(4, "c"));
        g.push_back(make_row(4, "d"));
        expect(texts(g) == strs { "c", "d" });
    };

    "push_back() (unwritten row) respects the limit"_test = []
    {
        grid g;
        g.set_rows_limit(2);
        g.push_back(make_row(4, "a"));
        g.push_back(make_row(4, "b"));
        g.push_back();
        expect(that % g.rows() == 2UZ);
        expect(texts(g) == strs { "b", "" });
    };

    "lowering the limit evicts immediately"_test = []
    {
        auto g = make_grid(make_row(4, "a"), make_row(4, "b"), make_row(4, "c"), make_row(4, "d"));
        g.set_rows_limit(2);
        expect(texts(g) == strs { "c", "d" });
    };

    "a limit above the row count evicts nothing"_test = []
    {
        auto g = make_grid(make_row(4, "a"), make_row(4, "b"));
        g.set_rows_limit(10);
        expect(texts(g) == strs { "a", "b" });
    };

    "limit equal to the row count evicts nothing"_test = []
    {
        auto g = make_grid(make_row(4, "a"), make_row(4, "b"));
        g.set_rows_limit(2);
        expect(that % g.rows() == 2UZ);
    };

    "a limit of zero empties the grid"_test = []
    {
        auto g = make_grid(make_row(4, "a"), make_row(4, "b"));
        g.set_rows_limit(0);
        expect(that % g.rows() == 0UZ);
    };
};


suite<"grid resize: guards"> resize_guard_suite = []
{
    "same width is a no-op"_test = []
    {
        auto g = make_grid(make_row(10, "hello"));
        g.resize(10);
        expect(that % g.rows() == 1UZ);
        expect(that % g.columns() == 10UZ);
        expect(texts(g) == strs { "hello" });
    };

    "widths below 2 are ignored"_test = []
    {
        auto g = make_grid(make_row(10, "hello"));
        g.resize(1);
        g.resize(0);
        expect(that % g.columns() == 10UZ);
        expect(texts(g) == strs { "hello" });
    };

    "resizing an empty grid is harmless"_test = []
    {
        grid g;
        g.resize(5);
        expect(that % g.rows() == 0UZ);
    };
};


suite<"grid resize: reflow"> reflow_suite = []
{
    "growing keeps unwrapped rows separate"_test = []
    {
        auto g = make_grid(make_row(10, "hello"), make_row(10, "world"));
        g.resize(20);
        expect(that % g.columns() == 20UZ);
        expect(texts(g) == strs { "hello", "world" });
        expect(that % row_at(g, 0).columns() == 20UZ);
    };

    "shrinking keeps a line that still fits on one row"_test = []
    {
        auto g = make_grid(make_row(10, "hi"));
        g.resize(5);
        expect(that % g.columns() == 5UZ);
        expect(texts(g) == strs { "hi" });
    };

    "shrinking splits a long line"_test = []
    {
        auto g = make_grid(make_row(10, "abcdefghij"));
        g.resize(4);
        expect(that % g.columns() == 4UZ);
        expect(texts(g) == strs { "abcd", "efgh", "ij" });
    };

    "split rows are marked wrapped except the last"_test = []
    {
        auto g = make_grid(make_row(10, "abcdefghij"));
        g.resize(4);
        expect(wrapped_flags(g) == std::vector<bool> { true, true, false });
    };

    "an exact multiple does not leave an empty trailing row"_test = []
    {
        auto g = make_grid(make_row(8, "abcdefgh"));
        g.resize(4);
        expect(texts(g) == strs { "abcd", "efgh" });
    };

    "wrapped rows rejoin when growing"_test = []
    {
        auto g = make_grid(make_row(5, "abcde", true), make_row(5, "fghij"));
        g.resize(10);
        expect(texts(g) == strs { "abcdefghij" });
    };

    "a line wrapped over three rows rejoins"_test = []
    {
        auto g = make_grid(make_row(4, "abcd", true), make_row(4, "efgh", true), make_row(4, "ij"));
        g.resize(12);
        expect(texts(g) == strs { "abcdefghij" });
    };

    "a rejoined line is not marked wrapped"_test = []
    {
        auto g = make_grid(make_row(5, "abcde", true), make_row(5, "fghij"));
        g.resize(10);
        expect(!row_at(g, 0).is_wrapped());
    };

    "wrapped rows only join with their own continuation"_test = []
    {
        auto g = make_grid(make_row(5, "abcde", true), make_row(5, "fg"), make_row(5, "xyz"));
        g.resize(10);
        expect(texts(g) == strs { "abcdefg", "xyz" });
    };

    "unwritten lines are preserved"_test = []
    {
        auto g = make_grid(make_row(10, "a"), make_row(10, ""), make_row(10, "b"));
        g.resize(4);
        expect(texts(g) == strs { "a", "", "b" });
    };

    "shrink then grow restores the original line"_test = []
    {
        auto g = make_grid(make_row(10, "abcdefghij"));
        g.resize(4);
        g.resize(10);
        expect(texts(g) == strs { "abcdefghij" });
        expect(!row_at(g, 0).is_wrapped());
    };

    "every resulting row and cell is dirty"_test = []
    {
        auto src = make_row(10, "abcdefghij");
        src.set_dirty(false);
        auto g = make_grid(std::move(src));

        g.resize(4);

        for (std::size_t i = 0; i < g.rows(); i++)
        {
            auto &r = row_at(g, i);
            expect(r.is_dirty());
            for (const auto &c : r) expect(!c.attribute.has(cell::attributes::clean));
        }
    };

    "consecutive resizes keep reflowing from the current state"_test = []
    {
        auto g = make_grid(make_row(10, "abcdefghij"));
        g.resize(4);
        g.resize(6);
        expect(texts(g) == strs { "abcdef", "ghij" });
    };

    "resize does not leak state between grids"_test = []
    {
        auto a = make_grid(make_row(10, "abcdefghij"));
        a.resize(4);

        auto b = make_grid(make_row(10, "x"));
        b.resize(5);

        expect(texts(b) == strs { "x" });
        expect(texts(a) == strs { "abcd", "efgh", "ij" });
    };

    /* Shrinking can only add rows, which can push the grid past its limit. */
    "resize respects the rows limit"_test = []
    {
        auto g = make_grid(make_row(10, "abcdefghij"), make_row(10, "abcdefghij"),
                           make_row(10, "abcdefghij"));
        g.set_rows_limit(3);
        g.resize(5);

        expect(that % g.rows() == 3UZ) << "rewrapped rows must be trimmed to the limit";
        expect(texts(g) == strs { "fghij", "abcde", "fghij" }) << "oldest rows go first";
    };
};


suite<"grid resize: wide characters"> wide_suite = []
{
    auto make_wide = []
    {
        /* a b 世 _ c  (世 is two cells wide: head + spacer) */
        row r { 5 };
        put(r, 0, character::codepoint(U'a'));
        put(r, 1, character::codepoint(U'b'));
        put(r, 2, character::codepoint(U'\u4E16'));
        put(r, 3, character::spacer());
        put(r, 4, character::codepoint(U'c'));
        return r;
    };

    const std::string wide = "\xE4\xB8\x96";

    "a wide character that does not fit moves to the next row"_test = [&]
    {
        auto g = make_grid(make_wide());
        g.resize(3);

        expect(that % g.rows() == 2UZ);
        expect(texts(g) == strs { "ab", wide + "c" });
    };

    "the vacated cell is padding, marked unwritten"_test = [&]
    {
        auto g = make_grid(make_wide());
        g.resize(3);

        expect(cell_at(row_at(g, 0), 2).attribute.has(cell::attributes::unwritten));
        expect(!cell_at(row_at(g, 1), 0).attribute.has(cell::attributes::unwritten));
    };

    "a wide character never has its spacer split from its head"_test = [&]
    {
        auto g = make_grid(make_wide());
        g.resize(3);

        auto &second = row_at(g, 1);
        expect(cell_at(second, 0).content == character::codepoint(U'\u4E16'));
        expect(cell_at(second, 1).content.kind() == character::kind::spacer);
    };

    "two wide characters at width 3"_test = [&]
    {
        row r { 4 };
        put(r, 0, character::codepoint(U'\u4E16'));
        put(r, 1, character::spacer());
        put(r, 2, character::codepoint(U'\u4E16'));
        put(r, 3, character::spacer());

        auto g = make_grid(std::move(r));
        g.resize(3);

        expect(texts(g) == strs { wide, wide });
        expect(cell_at(row_at(g, 0), 2).attribute.has(cell::attributes::unwritten));
    };

    "a wide character that fits exactly is not padded"_test = [&]
    {
        row r { 4 };
        put(r, 0, character::codepoint(U'\u4E16'));
        put(r, 1, character::spacer());
        put(r, 2, character::codepoint(U'\u4E16'));
        put(r, 3, character::spacer());

        auto g = make_grid(std::move(r));
        g.resize(2);

        expect(texts(g) == strs { wide, wide });
        expect(cell_at(row_at(g, 0), 1).content.kind() == character::kind::spacer);
        expect(!cell_at(row_at(g, 0), 1).attribute.has(cell::attributes::unwritten));
    };

    "padding left by a wide character is dropped when a wrapped row is resized again"_test = []
    {
        auto g = make_grid(make_row(5, "abcd", true), make_row(5, "ef"));
        g.resize(10);
        expect(texts(g) == strs { "abcdef" });
    };
};


suite<"grid resize: row extras"> extras_suite = []
{
    "a uri is split across the new rows"_test = []
    {
        auto src = make_row(10, "abcdefghij");
        expect(src.add_uri(uri(2, 8)).has_value());

        auto g = make_grid(std::move(src));
        g.resize(5);

        expect(bounds(row_at(g, 0).uris())
               == bounds_t {
                   { 2, 5 }
        });
        expect(bounds(row_at(g, 1).uris())
               == bounds_t {
                   { 0, 3 }
        });
    };

    "split uri pieces keep their payload"_test = []
    {
        auto src = make_row(10, "abcdefghij");
        expect(src.add_uri(uri(2, 8, "http://x", 42)).has_value());

        auto g = make_grid(std::move(src));
        g.resize(5);

        for (std::size_t i = 0; i < 2; i++)
        {
            const auto u = *row_at(g, i).uris();
            expect(that % u.size() == 1UZ);
            expect(u[0].uri == "http://x");
            expect(u[0].id == 42U);
        }
    };

    "rows a uri does not touch get none"_test = []
    {
        auto src = make_row(10, "abcdefghij");
        expect(src.add_uri(uri(7, 9)).has_value());

        auto g = make_grid(std::move(src));
        g.resize(5);

        expect(bounds(row_at(g, 0).uris()).empty());
        expect(bounds(row_at(g, 1).uris())
               == bounds_t {
                   { 2, 4 }
        });
    };

    "a uri spanning wrapped rows becomes one range when growing"_test = []
    {
        auto a = make_row(5, "abcde", true);
        auto b = make_row(5, "fghij");
        expect(a.add_uri(uri(2, 5)).has_value());
        expect(b.add_uri(uri(0, 3)).has_value());

        auto g = make_grid(std::move(a), std::move(b));
        g.resize(10);

        expect(bounds(row_at(g, 0).uris())
               == bounds_t {
                   { 2, 8 }
        });
    };

    "adjacent uris with different ids stay separate when joined"_test = []
    {
        auto a = make_row(5, "abcde", true);
        auto b = make_row(5, "fghij");
        expect(a.add_uri(uri(2, 5, "http://a", 1)).has_value());
        expect(b.add_uri(uri(0, 3, "http://a", 2)).has_value());

        auto g = make_grid(std::move(a), std::move(b));
        g.resize(10);

        expect(bounds(row_at(g, 0).uris())
               == bounds_t {
                   { 2, 5 },
                   { 5, 8 }
        });
    };

    "uri round trip"_test = []
    {
        auto src = make_row(10, "abcdefghij");
        expect(src.add_uri(uri(2, 8)).has_value());

        auto g = make_grid(std::move(src));
        g.resize(4);
        g.resize(10);

        expect(bounds(row_at(g, 0).uris())
               == bounds_t {
                   { 2, 8 }
        });
    };

    "an underline is split across the new rows"_test = []
    {
        auto src = make_row(10, "abcdefghij");
        expect(src.add_underline(underline(1, 9)).has_value());

        auto g = make_grid(std::move(src));
        g.resize(4);

        expect(that % g.rows() == 3UZ);
        expect(bounds(row_at(g, 0).underlines())
               == bounds_t {
                   { 1, 4 }
        });
        expect(bounds(row_at(g, 1).underlines())
               == bounds_t {
                   { 0, 4 }
        });
        expect(bounds(row_at(g, 2).underlines())
               == bounds_t {
                   { 0, 1 }
        });
    };

    "underlines with different styles stay separate when joined"_test = []
    {
        auto a = make_row(5, "abcde", true);
        auto b = make_row(5, "fghij");
        expect(a.add_underline(underline(0, 5, 1)).has_value());
        expect(b.add_underline(underline(0, 5, 2)).has_value());

        auto g = make_grid(std::move(a), std::move(b));
        g.resize(10);

        expect(bounds(row_at(g, 0).underlines())
               == bounds_t {
                   { 0, 5  },
                   { 5, 10 }
        });
    };

    "underlines with the same style merge when joined"_test = []
    {
        auto a = make_row(5, "abcde", true);
        auto b = make_row(5, "fghij");
        expect(a.add_underline(underline(0, 5, 1)).has_value());
        expect(b.add_underline(underline(0, 5, 1)).has_value());

        auto g = make_grid(std::move(a), std::move(b));
        g.resize(10);

        expect(bounds(row_at(g, 0).underlines())
               == bounds_t {
                   { 0, 10 }
        });
    };

    "rows without extras get none"_test = []
    {
        auto g = make_grid(make_row(10, "abcdefghij"));
        g.resize(5);
        expect(bounds(row_at(g, 0).uris()).empty());
        expect(bounds(row_at(g, 0).underlines()).empty());
    };
};


suite<"grid resize: prompt range"> prompt_suite = []
{
    "the prompt range is split across the new rows"_test = []
    {
        auto src = make_row(10, "abcdefghij");
        src.set_prompt_row(row::range { 3, 8 });

        auto g = make_grid(std::move(src));
        g.resize(5);

        auto first  = row_at(g, 0).prompt_row();
        auto second = row_at(g, 1).prompt_row();
        expect(first.has_value() and *first == row::range { 3, 5 });
        expect(second.has_value() and *second == row::range { 0, 3 });
    };

    "rows outside the prompt are not prompt rows"_test = []
    {
        auto src = make_row(10, "abcdefghij");
        src.set_prompt_row(row::range { 0, 2 });

        auto g = make_grid(std::move(src));
        g.resize(5);

        expect(!row_at(g, 1).prompt_row());
    };

    "rows containing the prompt are flagged as prompt rows"_test = []
    {
        auto src = make_row(10, "abcdefghij");
        src.set_prompt_row(row::range { 0, 2 });

        auto g = make_grid(std::move(src));
        g.resize(5);

        expect(row_at(g, 0).prompt_row());
    };

    "a prompt spanning wrapped rows is merged when growing"_test = []
    {
        auto a = make_row(5, "abcde", true);
        auto b = make_row(5, "fghij");
        a.set_prompt_row(row::range { 3, 5 });
        b.set_prompt_row(row::range { 0, 3 });

        auto g = make_grid(std::move(a), std::move(b));
        g.resize(10);

        auto p = row_at(g, 0).prompt_row();
        expect(p.has_value() and *p == row::range { 3, 8 });
    };
};


suite<"grid resize: unwritten cells"> blank_suite = []
{
    "untouched rows do not multiply when shrinking"_test = []
    {
        auto g = make_grid(row { 10 }, row { 10 }, row { 10 });
        g.resize(5);
        expect(that % g.rows() == 3UZ);
        expect(that % g.columns() == 5UZ);
    };

    "untouched rows keep their count when growing"_test = []
    {
        auto g = make_grid(row { 10 }, row { 10 }, row { 10 });
        g.resize(20);
        expect(that % g.rows() == 3UZ);
        expect(that % g.columns() == 20UZ);
    };

    "written trailing spaces are content, not unwritten"_test = []
    {
        /* "ab" followed by four *written* spaces: 6 cells of content, so 2 rows at width 4 */
        auto g = make_grid(make_row(10, "ab    "));
        g.resize(4);
        expect(that % g.rows() == 2UZ);
    };

    "written cells stay un-unwritten, the rest stay unwritten"_test = []
    {
        auto g = make_grid(make_row(10, "ab"));
        g.resize(5);

        auto &r = row_at(g, 0);
        expect(!cell_at(r, 0).attribute.has(cell::attributes::unwritten));
        expect(!cell_at(r, 1).attribute.has(cell::attributes::unwritten));
        expect(cell_at(r, 2).attribute.has(cell::attributes::unwritten));
        expect(cell_at(r, 4).attribute.has(cell::attributes::unwritten));
    };
};


auto main() -> int {}