#pragma once
#include <cstdint>
#include <format>


namespace cart::term
{
    struct position
    {
        std::uint32_t column;
        std::uint32_t row;
    };


    struct range
    {
        position begin;
        position end;
    };


    enum class underline_style : std::uint8_t
    {
        none,
        single,
        doubles,
        curly,
        dotted,
        dashed
    };
}


template <typename CharT>
struct std::formatter<cart::term::position, CharT>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext &ctx) -> ParseContext::iterator
    { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const cart::term::position &pos, FormatContext &ctx) const
    { return std::format_to(ctx.out(), "{}:{}", pos.column, pos.row); }
};


template <typename CharT>
struct std::formatter<cart::term::range, CharT>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext &ctx) -> ParseContext::iterator
    { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const cart::term::range &r, FormatContext &ctx) const
    { return std::format_to(ctx.out(), "[{}]->[{}]", r.begin, r.end); }
};
