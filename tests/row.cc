#include <cstdint>
#include <limits>

#include <boost/ut.hpp>

#include "terminal/row.hh"

using namespace boost::ut;
using cart::term::color;
using cart::term::row;
using cart::term::underline_style;

suite _ = [] /* NOLINT */
{
    "row initializes with the requested number of columns"_test = []
    {
        row r { 80 };

        expect(r.columns() == 80);
        expect(r.cells.size() == 80);
    };

    "row supports zero columns"_test = []
    {
        row r { 0 };

        expect(r.columns() == 0);
        expect(r.cells.empty());
    };

    "range to_end is the maximum uint32 value"_test
        = [] { expect(row::range::to_end == std::numeric_limits<std::uint32_t>::max()); };

    "attribute unset is the maximum uint32 value"_test
        = [] { expect(row::attributes::unset == std::numeric_limits<std::uint32_t>::max()); };

    "attribute flags have distinct bits"_test = []
    {
        using attributes = row::attributes;

        expect(static_cast<std::uint8_t>(attributes::wrapped) == 1);
        expect(static_cast<std::uint8_t>(attributes::clean) == 2);
        expect(static_cast<std::uint8_t>(attributes::prompt_row) == 4);
    };

    "prompt range defaults to unset"_test = []
    {
        row r { 80 };

        expect(r.attribute.prompt_range.begin == row::attributes::unset);
        expect(r.attribute.prompt_range.end == row::attributes::unset);
    };

    "resize truncates the row"_test = []
    {
        row r { 10 };

        r.resize(5);

        expect(r.columns() == 5);
        expect(r.cells.size() == 5);
    };

    "resize pads the row"_test = []
    {
        row r { 5 };

        r.resize(10);

        expect(r.columns() == 10);
        expect(r.cells.size() == 10);
    };

    "resize to the existing size preserves the column count"_test = []
    {
        row r { 10 };

        r.resize(10);

        expect(r.columns() == 10);
    };

    "erase preserves the row dimensions"_test = []
    {
        row r { 10 };

        r.erase();

        expect(r.columns() == 10);
    };

    "erase of a range preserves the row dimensions"_test = []
    {
        row r { 10 };

        r.erase(row::range { 2, 6 });

        expect(r.columns() == 10);
    };

    "URI insertion creates extras and records the range"_test = []
    {
        row r { 10 };

        r.put_uri(row::range { 2, 6 }, "https://example.com", 42);

        expect(r.extras != nullptr);
        expect(r.extras->uris.size() == 1);

        const auto &uri = r.extras->uris.front();

        expect(uri.begin == 2);
        expect(uri.end == 6);
        expect(uri.uri == "https://example.com");
        expect(uri.id == 42);
    };

    "underline insertion creates extras and records the range"_test = []
    {
        row r { 10 };

        const auto     fg    = color::make_default_fg();
        constexpr auto style = underline_style {};

        r.put_underline(row::range { 1, 4 }, fg, style);

        expect(r.extras != nullptr);
        expect(r.extras->underlines.size() == 1);

        const auto &underline = r.extras->underlines.front();

        expect(underline.begin == 1);
        expect(underline.end == 4);
        expect(underline.style == style);
    };

    "erase_uris removes a URI range"_test = []
    {
        row r { 10 };

        r.put_uri(row::range { 2, 6 }, "https://example.com", 42);
        r.erase_uris(row::range { 2, 6 });

        expect(r.extras != nullptr);
        expect(r.extras->uris.empty());
    };

    "erase_underlines removes an underline range"_test = []
    {
        row r { 10 };

        r.put_underline(row::range { 1, 4 }, color::make_default_fg(), underline_style {});

        r.erase_underlines(row::range { 1, 4 });

        expect(r.extras != nullptr);
        expect(r.extras->underlines.empty());
    };

    "extras clear removes URI and underline metadata"_test = []
    {
        struct row::extras extras;

        extras.uris.push_back(row::uri_range {
            { 1, 4 },
            "https://example.com", 42
        });

        extras.underlines.push_back(row::underline_range {
            { 2, 5 },
            color::make_default_fg(), underline_style {}
        });

        extras.clear();

        expect(extras.uris.empty());
        expect(extras.underlines.empty());
    };
};

auto main() -> int { return 0; }

