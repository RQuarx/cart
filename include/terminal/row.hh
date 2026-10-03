#pragma once
#include <memory>
#include <string>
#include <vector>

#include "terminal/cell.hh"
#include "terminal/types.hh"


namespace cart::term
{
    struct row
    {
        struct range
        {
            static constexpr std::uint32_t to_end = std::numeric_limits<std::uint32_t>::max();

            std::uint32_t begin; /* points to the starting column */
            std::uint32_t end;   /* points to the end column + 1 */
        };


        struct uri_range : range
        {
            std::string uri;
            std::size_t id;
        };


        struct underline_range : range
        {
            color           fg;
            underline_style style;
        };


        struct extras
        {
            std::vector<uri_range>       uris;
            std::vector<underline_range> underlines;

            void clear();
        };


        class attribute final : public trait::attribute<std::uint8_t>
        {
        public:
            enum flag : std::uint8_t
            {
                wrapped    = 1 << 0,
                clean      = 1 << 1,
                prompt_row = 1 << 2,
            };

            static constexpr auto unset = std::numeric_limits<std::uint32_t>::max();

            range prompt_range { attribute::unset, attribute::unset };
        };


        std::vector<cell>       cells;
        attribute               attribute;
        std::unique_ptr<extras> extras;


        explicit constexpr row(std::size_t columns) : cells { columns } {}

        [[nodiscard]]
        constexpr auto columns() const noexcept -> std::size_t
        { return cells.size(); }

        /** @brief Force the renderer to redraw every cell of this row. */
        void damage() noexcept;

        /** @brief Blank all cells, drop all ranges, reset wrapped and osc133. */
        void erase(color bg = color::make_default_bg()) noexcept;

        /** @note May allocate when the call splits a range in two. */
        void erase(range range, color bg = color::make_default_bg());

        /** @brief Truncates or pads, no reflow. */
        void resize(std::size_t column);


        void put_uri(range range, std::string_view uri, std::size_t id);
        void put_underline(range range, color color, underline_style style);
        void erase_uris(range range);
        void erase_underlines(range range);
    };
}
