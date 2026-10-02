#pragma once
#include "terminal/color.hh"


namespace cart::term
{
    struct cell
    {
        class attribute
        {
        public:
            enum flag : std::uint16_t
            {
                bold          = 1 << 0,
                dim           = 1 << 1,
                italic        = 1 << 2,
                underline     = 1 << 3,
                blinking      = 1 << 4,
                inverse       = 1 << 5,
                hidden        = 1 << 6,
                strikethrough = 1 << 7,

                clean    = 1 << 8,
                selected = 1 << 9,
                confined = 1 << 10,
                url      = 1 << 11,
            };


            [[nodiscard]]
            constexpr auto has(flag f) const noexcept -> bool
            { return (m_bits & f) != 0; }

            constexpr void set(flag f, bool state) noexcept
            { state ? m_bits |= f : m_bits &= std::uint16_t(~f); }

        private:
            std::uint16_t m_bits = 0;
        } attribute;


        /**
         * A cell's content, packed into 32 bits.
         *
         * - [ 0x00000000 ... 0x0010FFFF ]: plain unicode code point (0 = empty)
         * - [ 0x00200000 ... 0x4010FFFF ]: index into a side table of composed characters
         *                                  (base + combining marks, grapheme clusters)
         * - 0x40100000: spacer (trailing half of a wide glyph)
         */
        class character
        {
        public:
            enum class kind : std::uint8_t
            {
                codepoint,
                composed,
                spacer,
            };


            constexpr character() noexcept = default;


            [[nodiscard]]
            static constexpr auto make_codepoint(char32_t cp) noexcept -> character
            {
                /* anything above the unicode range would collide with the
                   composed range, so substitute U+FFFD. */
                return { cp > character::max_codepoint ? character::replacement
                                                       : std::uint32_t(cp) };
            }

            /** index must be <= max_composed_index */
            [[nodiscard]]
            static constexpr auto make_composed(std::uint32_t index) noexcept -> character
            { return { character::composed_lo + (index & character::max_composed_index) }; }

            [[nodiscard]]
            static constexpr auto make_spacer() noexcept -> character
            { return { character::spacer_value }; }


            [[nodiscard]]
            constexpr auto get_kind() const noexcept -> kind
            {
                if (m_value <= character::max_codepoint) return kind::codepoint;
                if (m_value == character::spacer_value) return kind::spacer;
                return kind::composed;
            }

            [[nodiscard]]
            constexpr auto is_codepoint() const noexcept -> bool
            { return m_value <= character::max_codepoint; }

            [[nodiscard]]
            constexpr auto is_composed() const noexcept -> bool
            { return m_value >= character::composed_lo and m_value <= character::composed_hi; }

            [[nodiscard]]
            constexpr auto is_spacer() const noexcept -> bool
            { return m_value == character::spacer_value; }

            [[nodiscard]]
            constexpr auto is_empty() const noexcept -> bool
            { return m_value == 0; }


            /* Valid only if is_codepoint() == true */
            [[nodiscard]]
            constexpr auto get_codepoint() const noexcept -> char32_t
            { return char32_t(m_value); }

            /* Valid only if is_composed() == true */
            [[nodiscard]]
            constexpr auto get_composed_index() const noexcept -> std::uint32_t
            { return m_value - character::composed_lo; }


            [[nodiscard]]
            friend constexpr auto operator==(character, character) noexcept -> bool = default;


        private:
            std::uint32_t m_value = 0;


            constexpr character(std::uint32_t value) noexcept : m_value { value } {}


            static constexpr std::uint32_t max_codepoint = 0x0010FFFF;
            static constexpr std::uint32_t replacement   = 0xFFFD;

            static constexpr std::uint32_t composed_lo        = 0x00200000;
            static constexpr std::uint32_t max_composed_index = 0x3FFFFFFF;
            static constexpr std::uint32_t composed_hi        = composed_lo + max_composed_index;

            static constexpr std::uint32_t spacer_value = composed_hi + 1;
        } character;


        struct color
        {
            term::color bg = term::color::make_default_bg();
            term::color fg = term::color::make_default_fg();


            [[nodiscard]]
            constexpr auto inverse() const noexcept -> color
            {
                color c = *this;
                std::swap(c.bg, c.fg);
                return c;
            }
        } color;

        /* 2 bytes leftover... extra attributes or stuff can be added */
    };


    static_assert(sizeof(cell) == 16, "Too big of a size of cart::term:cell.");
}
