#pragma once
#include "core/color.hh"


namespace cart::core
{
    struct cell
    {
        class attibute
        {
        public:
            enum flag : std::uint8_t
            {
                bold          = 1 << 0,
                dim           = 1 << 1,
                italic        = 1 << 2,
                underline     = 1 << 3,
                blinking      = 1 << 4,
                inverse       = 1 << 5,
                hidden        = 1 << 6,
                strikethrough = 1 << 7,
            };


            [[nodiscard]]
            constexpr auto has(flag f) const noexcept -> bool
            { return (bits & f) != 0; }

            constexpr void set(flag f, bool state) noexcept { state ? (bits |= f) : (bits &= ~f); }

        private:
            std::uint8_t bits = 0;
        };


        struct color
        {
            core::color bg = core::color::make_default_bg();
            core::color fg = core::color::make_default_fg();


            [[nodiscard]]
            constexpr auto inverse() const noexcept -> color
            {
                color c = *this;
                std::swap(c.bg, c.fg);
                return c;
            }
        };


        cell::color    color;     /* 8 bytes */
        char32_t       glyph;     /* 4 bytes */
        cell::attibute attribute; /* 1 bytes */
        std::uint8_t   width;     /* 1 bytes + 2 alignment */
    };
}
