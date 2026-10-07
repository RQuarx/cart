#pragma once
#include "shared/traits.hh"
#include "terminal/color.hh"


namespace cart::term
{
    struct cell
    {
        class attribute final : public trait::attribute<std::uint16_t>
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

            std::uint8_t width = 0;
        };


        /**
         * A cell's content, packed into 32 bits.
         *
         * - 0x00000000 - 0x0010FFFF  codepoint
         * - 0x00110000 - 0x001FFFFF  unused
         * - 0x00200000 - 0x401FFFFF  composed
         * - 0x40200000 - 0xFFFFFFFF  spacer
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
            static constexpr auto codepoint(char32_t cp) noexcept -> character
            {
                /* anything above the unicode range would collide with the
                   composed range, so substitute U+FFFD. */
                return { cp > character::max_codepoint ? character::replacement
                                                       : std::uint32_t(cp) };
            }

            /** index must be <= max_composed_index */
            [[nodiscard]]
            static constexpr auto composed(std::uint32_t index) noexcept -> character
            { return { character::composed_lo + (index & character::max_composed_index) }; }

            [[nodiscard]]
            static constexpr auto spacer(std::uint32_t remaining = 0) noexcept -> character
            { return { character::spacer_base + remaining }; }


            [[nodiscard]]
            constexpr auto get_kind() const noexcept -> kind
            {
                if (m_value <= max_codepoint) return kind::codepoint;
                if (m_value >= spacer_base) return kind::spacer;
                return kind::composed;
            }

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

            /* Valid only if is_spacer() == true */
            [[nodiscard]]
            constexpr auto get_spacer_remaining() const noexcept -> std::uint32_t
            { return m_value - spacer_base; }


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

            static constexpr std::uint32_t spacer_base = composed_hi + 1;
        };


        struct colors
        {
            term::color bg = term::color::make_default_bg();
            term::color fg = term::color::make_default_fg();


            [[nodiscard]]
            constexpr auto inverse() const noexcept -> colors
            {
                colors c = *this;
                std::swap(c.bg, c.fg);
                return c;
            }
        };


        character character = character::codepoint(U' ');
        attribute attribute;
        colors    colors;

        /* 2 bytes leftover... extra attributes or stuff can be added */
    };


    static_assert(sizeof(cell) == 16, "Too big of a size of cart::term:cell.");
}
