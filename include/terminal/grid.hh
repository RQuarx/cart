#pragma once
#include <cstddef>
#include <deque>

#include "shared/result.hh"
#include "terminal/row.hh"


namespace cart::term
{
    namespace _impl { struct logical_line; }


    class grid
    {
    public:
        [[nodiscard]] auto columns() const noexcept -> std::size_t;
        [[nodiscard]] auto rows() const noexcept -> std::size_t;


        [[nodiscard]] auto operator[](std::size_t row) noexcept -> result<term::row *>;
        [[nodiscard]] auto operator[](std::size_t row) const noexcept -> result<const term::row *>;


        [[nodiscard]]
        auto operator[](std::size_t begin, std::size_t end) noexcept
            -> result<std::ranges::subrange<std::deque<term::row>::iterator>>;

        [[nodiscard]]
        auto operator[](std::size_t begin, std::size_t end) const noexcept
            -> result<std::ranges::subrange<std::deque<term::row>::const_iterator>>;


        /**
         * The resize doesn't take in `new_rows` because rows amount is not really important to the
         * storage it self, and is a variable that the viewport handle.
         */
        void resize(std::size_t new_columns);
        void pop_front();
        void push_back(row r);
        void push_back();

        void set_rows_limit(std::size_t n);

    private:
        std::deque<term::row> m_rows;
        std::size_t           m_rows_limit = 100'000;


        void mf_ensure_rows_is_under_limit();

        static void
        rewrap(_impl::logical_line &line, std::size_t new_columns, std::deque<term::row> &out_rows);


        /* Copies every range that overlaps [pos, end) and rebases it to the chunk. */
        static void clip(const auto &src, std::uint32_t pos, std::uint32_t end, auto &&add)
        {
            for (const auto &r : src)
            {
                if (r.end <= pos or r.begin >= end) continue;
                auto c  = r;
                c.begin = std::max(r.begin, pos) - pos;
                c.end   = std::min(r.end, end) - pos;
                add(std::move(c));
            }
        }
    };
}
