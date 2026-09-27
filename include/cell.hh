#pragma once
#include "color.hh"


namespace cart
{
    class cell_attribute
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


        [[nodiscard]] auto has(flag f) const noexcept -> bool;

        void set(flag f, bool state) noexcept;
        void apply_from_other(const cell_attribute &other) noexcept;

    private:
        std::uint8_t bits = 0;
    };


    class cell_color
    {
    public:
        void set_background(color color) noexcept;
        void set_foreground(color color) noexcept;

        [[nodiscard]] auto get_background() const noexcept -> color;
        [[nodiscard]] auto get_foreground() const noexcept -> color;
        [[nodiscard]] auto inverse() const noexcept -> cell_color;

        void apply_from_other(const cell_color &other) noexcept;

    private:
        color bg;
        color fg;
    };


    struct cell
    {
        cell_color     color;     /* 8 bytes */
        char32_t       glyph;     /* 4 bytes */
        cell_attribute attribute; /* 1 bytes */
        std::uint8_t   width;     /* 1 bytes + 2 alignment */
    };
}
