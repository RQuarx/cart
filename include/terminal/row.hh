#pragma once
#include <memory>
#include <string>
#include <vector>

#include <utf8.h>

#include "terminal/cell.hh"
#include "terminal/types.hh"


namespace cart::term
{
    namespace _impl
    {
        template <typename F>
        concept composed_resolver = std::is_invocable_r_v<std::string_view, F, std::uint32_t>;
    }


    class row
    {
    public:
        struct range
        {
            static constexpr auto unset = std::numeric_limits<std::uint32_t>::max();

            std::uint32_t begin = unset;
            std::uint32_t end   = unset;


            friend auto operator==(range, range) noexcept -> bool = default;

            /** Clamps `range::end` to `n`. */
            constexpr auto clamp(std::uint32_t n) noexcept -> range &
            {
                end = std::min(end, n);
                return *this;
            }
        };


        class extras
        {
        public:
            struct uri_range final : row::range
            {
                std::string uri;
                std::size_t id;


                friend auto operator==(const uri_range &a, const uri_range &b) noexcept -> bool
                { return a.id == b.id and a.uri == b.uri; }
            };


            struct underline_range final : row::range
            {
                color           fg;
                underline_style style;


                friend auto operator==(underline_range a, underline_range b) noexcept -> bool
                { return a.style == b.style and a.fg == b.fg; }
            };


            void clear();

            [[nodiscard]] auto uris() const noexcept -> std::optional<std::span<const uri_range>>;
            [[nodiscard]] auto underlines() const noexcept
                -> std::optional<std::span<const underline_range>>;

            void add_uri(uri_range range);
            void add_underline(underline_range range);

            void erase_uri(row::range range);
            void erase_underline(row::range range);

        private:
            using range_pair = std::pair<std::vector<uri_range>, std::vector<underline_range>>;

            std::unique_ptr<range_pair> m_data;


            [[nodiscard]] auto mf_uris() noexcept -> std::optional<std::vector<uri_range> *>;
            [[nodiscard]] auto mf_underlines() noexcept
                -> std::optional<std::vector<underline_range> *>;
        };


        struct attributes final : public trait::attribute<std::uint8_t>
        {
            enum flag : std::uint8_t
            {
                wrapped    = 1 << 0,
                clean      = 1 << 1,
                prompt_row = 1 << 2,
            };


            row::range prompt_range;
        };

        using iterator               = std::vector<cell>::iterator;
        using const_iterator         = std::vector<cell>::const_iterator;
        using reverse_iterator       = std::vector<cell>::reverse_iterator;
        using const_reverse_iterator = std::vector<cell>::const_reverse_iterator;


        explicit row(std::size_t columns);

        [[nodiscard]] auto operator[](std::size_t col) noexcept -> result<cell *>;
        [[nodiscard]] auto operator[](std::size_t col) const noexcept -> result<const cell *>;

        [[nodiscard]] auto operator[](range r) noexcept -> result<std::span<cell>>;
        [[nodiscard]] auto operator[](range r) const noexcept -> result<std::span<const cell>>;

        [[nodiscard]] auto columns() const noexcept -> std::size_t;

        [[nodiscard]] auto is_wrapped() const noexcept -> bool;
        [[nodiscard]] auto is_dirty() const noexcept -> bool;
        [[nodiscard]] auto is_prompt_row() const noexcept -> bool;

        void set_wrapped(bool state = true) noexcept;

        /** Setting this row as dirty will also set all columns contained within this row as one. */
        void set_dirty(bool state = true) noexcept;

        /**
         * This method would set this row as a prompt row alongside the prompt range within the row.
         * If `prompt_range == std::nullopt`, this row would be unset as a prompt row,
         * and the prompt range would be erased.
         */
        void set_prompt_row(std::optional<row::range> prompt_range = std::nullopt) noexcept;


        template <_impl::composed_resolver F>
        void to_string(std::string &buffer, F &&resolver) const
        {
            buffer.clear();
            buffer.reserve(columns());

            for (const auto &c : *this) switch (c.content.kind())
                {
                case cell::character::kind::codepoint:
                    utf8::append(c.content.as_codepoint(), std::back_inserter(buffer));
                    break;

                case cell::character::kind::composed:
                    buffer += resolver(c.content.as_composed_index());
                    break;

                case cell::character::kind::spacer: break;
                };
        }


        template <_impl::composed_resolver F>
        auto to_string(F &&resolver) const -> std::string
        {
            std::string buffer;
            to_string(buffer, std::forward<F>(resolver));
            return buffer;
        }


        [[nodiscard]] constexpr auto begin() noexcept -> iterator { return m_columns.begin(); }
        [[nodiscard]] constexpr auto end() noexcept -> iterator { return m_columns.end(); }

        [[nodiscard]]
        constexpr auto begin() const noexcept -> const_iterator
        { return m_columns.begin(); }

        [[nodiscard]]
        constexpr auto end() const noexcept -> const_iterator
        { return m_columns.end(); }


#pragma region "m_extras method"
        [[nodiscard]]
        auto uris() const noexcept -> std::optional<std::span<const extras::uri_range>>;

        [[nodiscard]]
        auto underlines() const noexcept -> std::optional<std::span<const extras::underline_range>>;

        auto add_uri(extras::uri_range range) noexcept -> result<>;
        auto add_underline(extras::underline_range range) noexcept -> result<>;

        void erase_uri(row::range range);
        void erase_underline(row::range range);
#pragma endregion


    private:
        std::vector<cell> m_columns;
        attributes        m_attribute;
        extras            m_extras;
    };
}
