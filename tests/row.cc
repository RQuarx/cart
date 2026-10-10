#include <utils.cc> /* NOLINT */


suite<"row::range"> range_suite = []
{
    "unset is the max value"_test = []
    {
        row::range r;
        expect(r.begin == row::range::unset);
        expect(r.end == row::range::unset);
    };

    "clamp reduces end"_test = []
    {
        row::range r { 2, 100 };
        r.clamp(10);
        expect(that % r.begin == 2U);
        expect(that % r.end == 10U);
    };

    "clamp never extends end"_test = []
    {
        row::range r { 2, 5 };
        r.clamp(10);
        expect(that % r.end == 5U);
    };

    "clamp is chainable"_test = []
    {
        row::range r { 0, 100 };
        expect(&r.clamp(50) == &r);
    };

    "equality"_test = []
    {
        expect(row::range { 1, 2 } == row::range { 1, 2 });
        expect(row::range { 1, 2 } != row::range { 1, 3 });
        expect(row::range { 0, 2 } != row::range { 1, 2 });
    };
};


suite<"row::extras uri"> uri_suite = []
{
    "fresh extras have no ranges"_test = []
    {
        row::extras e;
        expect(!e.uris().has_value());
        expect(!e.underlines().has_value());
    };

    "add creates storage"_test = []
    {
        row::extras e;
        e.add_uri(uri(0, 5));
        expect(e.uris().has_value());
        expect(bounds(e.uris())
               == bounds_t {
                   { 0, 5 }
        });
        /* adding a uri must not allocate/populate underlines contents */
        expect(bounds(e.underlines()).empty());
    };

    "ranges are kept sorted"_test = []
    {
        row::extras e;
        e.add_uri(uri(10, 15, "http://b", 2));
        e.add_uri(uri(0, 5, "http://a", 1));
        e.add_uri(uri(6, 8, "http://c", 3));
        expect(bounds(e.uris())
               == bounds_t {
                   { 0,  5  },
                   { 6,  8  },
                   { 10, 15 }
        });
    };

    "same payload, gap between: not merged"_test = []
    {
        row::extras e;
        e.add_uri(uri(0, 3));
        e.add_uri(uri(5, 8));
        expect(bounds(e.uris())
               == bounds_t {
                   { 0, 3 },
                   { 5, 8 }
        });
    };

    "adjacent with same payload merges (append)"_test = []
    {
        row::extras e;
        e.add_uri(uri(0, 3));
        e.add_uri(uri(3, 6));
        expect(bounds(e.uris())
               == bounds_t {
                   { 0, 6 }
        });
    };

    "adjacent with same payload merges (prepend)"_test = []
    {
        row::extras e;
        e.add_uri(uri(3, 6));
        e.add_uri(uri(0, 3));
        expect(bounds(e.uris())
               == bounds_t {
                   { 0, 6 }
        });
    };

    "new range bridging two same-payload ranges merges all three"_test = []
    {
        row::extras e;
        e.add_uri(uri(0, 3));
        e.add_uri(uri(6, 9));
        e.add_uri(uri(3, 6));
        expect(bounds(e.uris())
               == bounds_t {
                   { 0, 9 }
        });
    };

    "adjacent with different id: not merged"_test = []
    {
        row::extras e;
        e.add_uri(uri(0, 3, "http://a", 1));
        e.add_uri(uri(3, 6, "http://a", 2));
        expect(bounds(e.uris())
               == bounds_t {
                   { 0, 3 },
                   { 3, 6 }
        });
    };

    "adjacent with different uri: not merged"_test = []
    {
        row::extras e;
        e.add_uri(uri(0, 3, "http://a", 1));
        e.add_uri(uri(3, 6, "http://b", 1));
        expect(bounds(e.uris())
               == bounds_t {
                   { 0, 3 },
                   { 3, 6 }
        });
    };

    "overlapping same payload coalesces"_test = []
    {
        row::extras e;
        e.add_uri(uri(0, 5));
        e.add_uri(uri(3, 8));
        expect(bounds(e.uris())
               == bounds_t {
                   { 0, 8 }
        });
    };

    "new range strictly inside splits the old one"_test = []
    {
        row::extras e;
        e.add_uri(uri(0, 10, "http://a", 1));
        e.add_uri(uri(3, 6, "http://b", 2));

        expect(bounds(e.uris())
               == bounds_t {
                   { 0, 3  },
                   { 3, 6  },
                   { 6, 10 }
        });

        const auto s = *e.uris();
        expect(s[0].id == 1U and s[1].id == 2U and s[2].id == 1U);
        expect(s[0].uri == "http://a" and s[2].uri == "http://a");
    };

    "new range overwrites partially overlapped neighbours"_test = []
    {
        row::extras e;
        e.add_uri(uri(0, 5, "http://a", 1));
        e.add_uri(uri(5, 10, "http://b", 2));
        e.add_uri(uri(3, 7, "http://c", 3));
        expect(bounds(e.uris())
               == bounds_t {
                   { 0, 3  },
                   { 3, 7  },
                   { 7, 10 }
        });
    };

    "new range covering several replaces them all"_test = []
    {
        row::extras e;
        e.add_uri(uri(0, 3, "http://a", 1));
        e.add_uri(uri(4, 6, "http://b", 2));
        e.add_uri(uri(8, 9, "http://c", 3));
        e.add_uri(uri(0, 10, "http://d", 4));
        expect(bounds(e.uris())
               == bounds_t {
                   { 0, 10 }
        });
        expect((*e.uris())[0].id == 4U);
    };

    "re-adding an identical range is idempotent"_test = []
    {
        row::extras e;
        e.add_uri(uri(2, 7));
        e.add_uri(uri(2, 7));
        expect(bounds(e.uris())
               == bounds_t {
                   { 2, 7 }
        });
    };
};


suite<"row::extras erase"> erase_suite = []
{
    "erase on empty extras is a no-op"_test = []
    {
        row::extras e;
        e.erase_uri({ 0, 10 });
        e.erase_underline({ 0, 10 });
        expect(!e.uris().has_value());
        expect(!e.underlines().has_value());
    };

    "erase outside any range is a no-op"_test = []
    {
        row::extras e;
        e.add_uri(uri(5, 8));
        e.erase_uri({ 0, 5 });
        e.erase_uri({ 8, 20 });
        expect(bounds(e.uris())
               == bounds_t {
                   { 5, 8 }
        });
    };

    "erase exactly a range removes it"_test = []
    {
        row::extras e;
        e.add_uri(uri(2, 6));
        e.erase_uri({ 2, 6 });
        expect(bounds(e.uris()).empty());
    };

    "erase a superset removes the range"_test = []
    {
        row::extras e;
        e.add_uri(uri(2, 6));
        e.erase_uri({ 0, 100 });
        expect(bounds(e.uris()).empty());
    };

    "erase clips the right side"_test = []
    {
        row::extras e;
        e.add_uri(uri(0, 10));
        e.erase_uri({ 6, 20 });
        expect(bounds(e.uris())
               == bounds_t {
                   { 0, 6 }
        });
    };

    "erase clips the left side"_test = []
    {
        row::extras e;
        e.add_uri(uri(4, 10));
        e.erase_uri({ 0, 6 });
        expect(bounds(e.uris())
               == bounds_t {
                   { 6, 10 }
        });
    };

    "erase aligned to the left edge shrinks from the left"_test = []
    {
        row::extras e;
        e.add_uri(uri(0, 10));
        e.erase_uri({ 0, 4 });
        expect(bounds(e.uris())
               == bounds_t {
                   { 4, 10 }
        });
    };

    "erase aligned to the right edge shrinks from the right"_test = []
    {
        row::extras e;
        e.add_uri(uri(0, 10));
        e.erase_uri({ 4, 10 });
        expect(bounds(e.uris())
               == bounds_t {
                   { 0, 4 }
        });
    };

    "erase strictly inside splits and keeps payload"_test = []
    {
        row::extras e;
        e.add_uri(uri(0, 10, "http://x", 42));
        e.erase_uri({ 3, 6 });

        expect(bounds(e.uris())
               == bounds_t {
                   { 0, 3  },
                   { 6, 10 }
        });
        for (const auto &r : *e.uris())
        {
            expect(r.uri == "http://x");
            expect(r.id == 42U);
        }
    };

    "erase spanning several ranges"_test = []
    {
        row::extras e;
        e.add_uri(uri(0, 3, "http://a", 1));
        e.add_uri(uri(5, 8, "http://b", 2));
        e.add_uri(uri(10, 13, "http://c", 3));
        e.erase_uri({ 2, 11 });
        expect(bounds(e.uris())
               == bounds_t {
                   { 0,  2  },
                   { 11, 13 }
        });
    };

    "erase touching but not overlapping is a no-op"_test = []
    {
        row::extras e;
        e.add_uri(uri(3, 6));
        e.erase_uri({ 6, 9 });
        e.erase_uri({ 0, 3 });
        expect(bounds(e.uris())
               == bounds_t {
                   { 3, 6 }
        });
    };

    "erase_uri leaves underlines untouched"_test = []
    {
        row::extras e;
        e.add_uri(uri(0, 10));
        e.add_underline(underline(0, 10));
        e.erase_uri({ 0, 10 });
        expect(bounds(e.uris()).empty());
        expect(bounds(e.underlines())
               == bounds_t {
                   { 0, 10 }
        });
    };

    "erase_underline leaves uris untouched"_test = []
    {
        row::extras e;
        e.add_uri(uri(0, 10));
        e.add_underline(underline(0, 10));
        e.erase_underline({ 0, 10 });
        expect(bounds(e.uris())
               == bounds_t {
                   { 0, 10 }
        });
        expect(bounds(e.underlines()).empty());
    };
};


suite<"row::extras underline"> underline_suite = []
{
    "same style, adjacent: merged"_test = []
    {
        row::extras e;
        e.add_underline(underline(0, 4, 1));
        e.add_underline(underline(4, 8, 1));
        expect(bounds(e.underlines())
               == bounds_t {
                   { 0, 8 }
        });
    };

    "different style, adjacent: not merged"_test = []
    {
        row::extras e;
        e.add_underline(underline(0, 4, 1));
        e.add_underline(underline(4, 8, 2));
        expect(bounds(e.underlines())
               == bounds_t {
                   { 0, 4 },
                   { 4, 8 }
        });
    };

    "different style overwrites the overlap"_test = []
    {
        row::extras e;
        e.add_underline(underline(0, 10, 1));
        e.add_underline(underline(4, 6, 2));
        expect(bounds(e.underlines())
               == bounds_t {
                   { 0, 4  },
                   { 4, 6  },
                   { 6, 10 }
        });
    };

    "sorted insertion"_test = []
    {
        row::extras e;
        e.add_underline(underline(10, 12, 1));
        e.add_underline(underline(0, 2, 1));
        expect(bounds(e.underlines())
               == bounds_t {
                   { 0,  2  },
                   { 10, 12 }
        });
    };

    "uri and underline ranges are independent"_test = []
    {
        row::extras e;
        e.add_uri(uri(0, 5));
        e.add_underline(underline(3, 9));
        expect(bounds(e.uris())
               == bounds_t {
                   { 0, 5 }
        });
        expect(bounds(e.underlines())
               == bounds_t {
                   { 3, 9 }
        });
    };

    "clear empties both lists but keeps storage"_test = []
    {
        row::extras e;
        e.add_uri(uri(0, 5));
        e.add_underline(underline(0, 5));
        e.clear();
        expect(e.uris().has_value() and e.uris()->empty());
        expect(e.underlines().has_value() and e.underlines()->empty());
    };

    "clear on fresh extras is a no-op"_test = []
    {
        row::extras e;
        e.clear();
        expect(!e.uris().has_value());
    };

    "usable again after clear"_test = []
    {
        row::extras e;
        e.add_uri(uri(0, 5));
        e.clear();
        e.add_uri(uri(2, 4));
        expect(bounds(e.uris())
               == bounds_t {
                   { 2, 4 }
        });
    };
};


suite<"row columns & access"> access_suite = []
{
    "columns() matches ctor"_test = []
    {
        expect(that % row { 80 }.columns() == 80UZ);
        expect(that % row { 0 }.columns() == 0UZ);
    };

    "iteration visits every cell"_test = []
    {
        row         r { 17 };
        std::size_t n = 0;
        for ([[maybe_unused]] auto &c : r) n++;
        expect(that % n == 17UZ);
        expect(that % std::size_t(r.end() - r.begin()) == 17UZ);
    };

    "operator[](col) in range"_test = []
    {
        row  r { 10 };
        auto c = r[3];
        expect(c.has_value());
        expect(*c == &*(r.begin() + 3));
    };

    "operator[](col) first and last"_test = []
    {
        row r { 10 };
        expect(r[0].has_value());
        expect(r[9].has_value());
    };

    "operator[](col) out of range is an error"_test = []
    {
        row r { 10 };
        expect(!r[10].has_value());
        expect(!r[1000].has_value());
    };

    "operator[](col) on a const row"_test = []
    {
        const row r { 4 };
        expect(r[2].has_value());
        expect(!r[4].has_value());
    };

    "operator[](range) returns the matching span"_test = []
    {
        row  r { 10 };
        auto s = r[row::range { 2, 6 }];
        expect(s.has_value());
        expect(that % s->size() == 4UZ);
        expect(s->data() == &*(r.begin() + 2));
    };

    "operator[](range) clamps end to columns"_test = []
    {
        row  r { 10 };
        auto s = r[row::range { 7, 500 }];
        expect(s.has_value());
        expect(that % s->size() == 3UZ);
    };

    "operator[](range) with open end (begin..unset)"_test = []
    {
        row  r { 10 };
        auto s = r[row::range { 4, row::range::unset }];
        expect(s.has_value());
        expect(that % s->size() == 6UZ);
    };

    "operator[](range) whole row"_test = []
    {
        row  r { 10 };
        auto s = r[row::range { 0, 10 }];
        expect(s.has_value() and s->size() == 10UZ);
    };

    "operator[](range) empty / inverted / unset is an error"_test = []
    {
        row r { 10 };
        expect(!r[row::range { 5, 5 }].has_value());
        expect(!r[row::range { 6, 2 }].has_value());
        expect(!r[row::range {}].has_value());
    };

    "operator[](range) on a const row"_test = []
    {
        const row r { 10 };
        auto      s = r[row::range { 1, 4 }];
        expect(s.has_value() and s->size() == 3UZ);
    };

    "operator[](range) begin past the end is an error"_test = []
    {
        row r { 5 };
        expect(!r[row::range { 10, 20 }].has_value());
    };
};


suite<"row flags"> flags_suite = []
{
    "defaults"_test = []
    {
        row r { 5 };
        expect(!r.is_wrapped());
        expect(!r.prompt_row());
    };

    "wrapped round trip"_test = []
    {
        row r { 5 };
        r.set_wrapped();
        expect(r.is_wrapped());
        r.set_wrapped(false);
        expect(!r.is_wrapped());
    };

    "dirty round trip"_test = []
    {
        row r { 5 };
        r.set_dirty(false);
        expect(!r.is_dirty());
        r.set_dirty(true);
        expect(r.is_dirty());
    };

    "prompt row round trip"_test = []
    {
        row r { 5 };
        r.set_prompt_row(row::range { 0, 2 });
        expect(r.prompt_row());
        r.set_prompt_row();
        expect(!r.prompt_row());
    };

    "set_dirty propagates to every cell"_test = []
    {
        row r { 6 };

        r.set_dirty(false);
        for (const auto &c : r) expect(c.attribute.has(cell::attributes::clean));

        r.set_dirty(true);
        for (const auto &c : r) expect(!c.attribute.has(cell::attributes::clean));
    };

    /* The flags below must be independent of each other. */
    "setting wrapped does not touch dirty / prompt"_test = []
    {
        row        r { 5 };
        const auto dirty = r.is_dirty();
        r.set_wrapped();
        expect(r.is_dirty() == dirty) << "wrapped must not change dirty";
        expect(!r.prompt_row()) << "wrapped must not make this a prompt row";
    };

    "setting prompt row does not touch wrapped / dirty"_test = []
    {
        row        r { 5 };
        const auto dirty = r.is_dirty();
        r.set_prompt_row(row::range { 0, 2 });
        expect(!r.is_wrapped()) << "prompt row must not make this wrapped";
        expect(r.is_dirty() == dirty) << "prompt row must not change dirty";
    };

    "setting dirty does not touch wrapped / prompt"_test = []
    {
        row r { 5 };
        r.set_dirty(false);
        expect(!r.is_wrapped()) << "clean must not make this wrapped";
        expect(!r.prompt_row()) << "clean must not make this a prompt row";
    };
};


suite<"row to_string"> to_string_suite = []
{
    "resolver satisfies the concept"_test = []
    {
        auto f = [](std::uint32_t) -> std::string_view { return ""; };
        static_assert(cart::term::_impl::composed_resolver<decltype(f)>);

        auto g = [](const std::string &) -> std::string_view { return ""; };
        static_assert(!cart::term::_impl::composed_resolver<decltype(g)>);
    };

    "buffer overload clears previous content"_test = []
    {
        row         r { 4 };
        std::string buf = "garbage-garbage-garbage";
        r.to_string(buf, [](std::uint32_t) -> std::string_view { return "?"; });
        expect(!buf.contains("garbage"));
    };

    "returning overload matches buffer overload"_test = []
    {
        row  r { 8 };
        auto resolver = [](std::uint32_t) -> std::string_view { return "?"; };

        std::string buf;
        r.to_string(buf, resolver);
        expect(r.to_string(resolver) == buf);
    };

    "zero-column row yields an empty string"_test = []
    {
        row r { 0 };
        expect(r.to_string([](std::uint32_t) -> std::string_view { return ""; }).empty());
    };

    "resolver is only invoked for composed cells"_test = []
    {
        /* Fresh cells are never composed clusters. */
        row r { 4 };
        int calls = 0;

        [[maybe_unused]] auto _ = r.to_string(
            [&](std::uint32_t) -> std::string_view
            {
                calls++;
                return "";
            });
        expect(that % calls == 0);
    };

    "default (empty) cells render no visible characters"_test = []
    {
        row  r { 4 };
        auto s = r.to_string([](std::uint32_t) -> std::string_view { return ""; });
        expect(s.find_first_not_of(std::string_view { " \0", 2 }) == std::string::npos);
    };


    "empty cells between content do not emit a NUL byte"_test = []
    {
        row r { 3 };
        (*r[0])->content = character::codepoint(U'a');
        /* r[1] stays empty */
        (*r[2])->content = character::codepoint(U'b');

        auto s = r.to_string([](std::uint32_t) -> std::string_view { return ""; });
        expect(!s.contains('\0'));
        expect(s.front() == 'a' and s.back() == 'b');
    };

    "ASCII codepoints are emitted as-is"_test = []
    {
        row r { 3 };
        (*r[0])->content = character::codepoint(U'h');
        (*r[1])->content = character::codepoint(U'i');
        (*r[2])->content = character::codepoint(U'!');
        expect(r.to_string([](std::uint32_t) -> std::string_view { return ""; }) == "hi!");
    };

    "multi-byte codepoints are UTF-8 encoded"_test = []
    {
        row r { 3 };
        (*r[0])->content = character::codepoint(U'\u00E9');     /* 2 bytes */
        (*r[1])->content = character::codepoint(U'\u4E16');     /* 3 bytes */
        (*r[2])->content = character::codepoint(U'\U0001F600'); /* 4 bytes */

        const std::string expected
            = std::string { "\xC3\xA9" } + "\xE4\xB8\x96" + "\xF0\x9F\x98\x80";
        expect(r.to_string([](std::uint32_t) -> std::string_view { return ""; }) == expected);
    };

    "spacer cells emit nothing (wide char tail)"_test = []
    {
        row r { 3 };
        (*r[0])->content = character::codepoint(U'\u4E16');
        (*r[1])->content = character::spacer();
        (*r[2])->content = character::codepoint(U'x');
        expect(r.to_string([](std::uint32_t) -> std::string_view { return ""; })
               == std::string { "\xE4\xB8\x96" } + "x");
    };

    "composed cells are replaced by the resolver's string"_test = []
    {
        row r { 3 };
        (*r[0])->content = character::codepoint(U'a');
        (*r[1])->content = character::composed(7);
        (*r[2])->content = character::codepoint(U'b');

        std::uint32_t seen = 0;
        auto          s    = r.to_string(
            [&](std::uint32_t idx) -> std::string_view
            {
                seen = idx;
                return "e\xCC\x81"; /* e + combining acute */
            });

        expect(that % seen == 7U);
        expect(s == std::string { "ae\xCC\x81" } + "b");
    };

    "composed index 0 is passed through unchanged"_test = []
    {
        row r { 1 };
        (*r[0])->content = character::composed(0);

        std::uint32_t seen = 99;
        r.to_string(
            [&](std::uint32_t idx) -> std::string_view
            {
                seen = idx;
                return "";
            });
        expect(that % seen == 0U);
    };

    "resolver is called once per composed cell, in order"_test = []
    {
        row r { 3 };
        (*r[0])->content = character::composed(1);
        (*r[1])->content = character::composed(2);
        (*r[2])->content = character::composed(3);

        std::vector<std::uint32_t> order;
        auto                       s = r.to_string(
            [&](std::uint32_t idx) -> std::string_view
            {
                order.push_back(idx);
                return "#";
            });
        expect(order == std::vector<std::uint32_t> { 1, 2, 3 });
        expect(s == "###");
    };
};


suite<"cell::character"> character_suite = []
{
    "default is empty U+0000 codepoint"_test = []
    {
        constexpr character c;
        expect(c.empty());
        expect(c.kind() == character::kind::codepoint);
        expect(c.as_codepoint() == U'\0');
    };

    "default cell is empty and unwritten"_test = []
    {
        cell c;
        expect(c.content.empty());
        expect(c.content == character {});
        expect(c.content == character::codepoint(0));
        expect(c.content.as_codepoint() == U'\0');
        expect(c.attribute.has(cell::attributes::unwritten));
        expect(that % c.attribute.width == 0);
    };

    "codepoint round trip"_test = []
    {
        for (char32_t cp : { U'A', U'\u00E9', U'\uFFFF', U'\U0001F600', U'\U0010FFFF' })
        {
            auto c = character::codepoint(cp);
            expect(c.kind() == character::kind::codepoint);
            expect(c.as_codepoint() == cp);
        }
    };

    "codepoint above the unicode range becomes U+FFFD"_test = []
    {
        expect(character::codepoint(char32_t(0x110000)) == character::codepoint(U'\uFFFD'));
        expect(character::codepoint(char32_t(0x7FFFFFFF)) == character::codepoint(U'\uFFFD'));
        expect(character::codepoint(char32_t(0xFFFFFFFF)).as_codepoint() == U'\uFFFD');
    };

    "max unicode codepoint is still a codepoint"_test = []
    {
        auto c = character::codepoint(char32_t(0x10FFFF));
        expect(c.kind() == character::kind::codepoint);
        expect(c.as_codepoint() == char32_t(0x10FFFF));
    };

    "composed round trip"_test = []
    {
        for (std::uint32_t idx : { 0U, 1U, 12345U, 0x3FFFFFFFU })
        {
            auto c = character::composed(idx);
            expect(c.kind() == character::kind::composed);
            expect(that % c.as_composed_index() == idx);
        }
    };

    "composed range never reaches the spacer range"_test = []
    {
        /* highest valid index sits right below the spacer base */
        expect(character::composed(0x3FFFFFFF).kind() == character::kind::composed);
        expect(character::spacer(0).kind() == character::kind::spacer);
    };

    "composed index is masked to its valid range"_test = []
    {
        expect(character::composed(0x40000000) == character::composed(0));
        expect(character::composed(0xFFFFFFFF) == character::composed(0x3FFFFFFF));
        expect(character::composed(0xFFFFFFFF).kind() == character::kind::composed);
    };

    "spacer defaults to zero remaining"_test = []
    {
        auto c = character::spacer();
        expect(c.kind() == character::kind::spacer);
        expect(that % c.as_spacer_remaining() == 0U);
    };

    "spacer round trip"_test = []
    {
        for (std::uint32_t n : { 0U, 1U, 5U, 1000U })
        {
            auto c = character::spacer(n);
            expect(c.kind() == character::kind::spacer);
            expect(that % c.as_spacer_remaining() == n);
        }
    };

    "default is empty, constructed characters are not"_test = []
    {
        expect(character {}.empty());
        expect(!character::codepoint(U' ').empty());
        expect(!character::codepoint(U'a').empty());
        expect(!character::composed(0).empty());
        expect(!character::spacer(0).empty());
    };

    "equality distinguishes kinds and values"_test = []
    {
        expect(character {} == character {});
        expect(character {} != character::codepoint(U' '));
        expect(character::codepoint(U'a') == character::codepoint(U'a'));
        expect(character::codepoint(U'a') != character::codepoint(U'b'));
        expect(character::composed(1) != character::composed(2));
        expect(character::composed(0) != character::codepoint(U'\0'));
        expect(character::spacer(0) != character::composed(0));
    };
};


suite<"cell::attributes"> cell_attributes_suite = []
{
    "no flag other than unwritten is set by default"_test = []
    {
        cell c;
        for (auto f :
             { cell::attributes::bold, cell::attributes::dim, cell::attributes::italic,
               cell::attributes::underline, cell::attributes::blinking, cell::attributes::inverse,
               cell::attributes::hidden, cell::attributes::strikethrough, cell::attributes::clean,
               cell::attributes::selected, cell::attributes::confined, cell::attributes::url })
            expect(!c.attribute.has(f));
    };

    "unwritten can be cleared and set again"_test = []
    {
        cell c;
        c.attribute.set(cell::attributes::unwritten, false);
        expect(!c.attribute.has(cell::attributes::unwritten));
        c.attribute.set(cell::attributes::unwritten, true);
        expect(c.attribute.has(cell::attributes::unwritten));
    };

    "unwritten is independent of the other flags"_test = []
    {
        cell c;
        c.attribute.set(cell::attributes::bold, true);
        expect(c.attribute.has(cell::attributes::bold));
        expect(c.attribute.has(cell::attributes::unwritten));

        c.attribute.set(cell::attributes::unwritten, false);
        expect(c.attribute.has(cell::attributes::bold));

        c.attribute.set(cell::attributes::bold, false);
        expect(!c.attribute.has(cell::attributes::unwritten));
    };

    "attribute copies keep unwritten"_test = []
    {
        cell a;
        cell b = a;
        expect(b.attribute.has(cell::attributes::unwritten));

        a.attribute.set(cell::attributes::unwritten, false);
        expect(b.attribute.has(cell::attributes::unwritten));
    };
};


auto main() -> int {}
