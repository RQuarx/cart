#include "terminal/row.hh"

using cart::term::row;


namespace
{
    /**
     * @brief Remove a range from a sorted, non-overlapping range list.
     *
     * A range that strictly contains the hole is split into two.
     */
    template <typename T>
    void erase_in(std::vector<T> &v, row::range range)
    {
        for (std::size_t i = 0; i < v.size();)
        {
            T &r = v[i];

            if (r.begin >= range.end) break; /* sorted: nothing further can overlap */
            if (r.end <= range.begin)        /* entirely left of the hole */
            {
                i++;
                continue;
            }

            if (r.begin < range.begin and r.end > range.end) /* hole is strictly inside: split */
            {
                T tail     = r;
                tail.begin = range.end;
                r.end      = range.begin;
                v.insert(v.begin() + static_cast<std::ptrdiff_t>(i) + 1, std::move(tail));
                break; /* r is invalidated; nothing further overlaps */
            }

            if (r.begin < range.begin) /* clip the right side */
            {
                r.end = range.begin;
                i++;
            }
            else if (r.end > range.end) /* clip the left side */
            {
                r.begin = range.end;
                i++;
            }
            else
                v.erase(v.begin() + static_cast<std::ptrdiff_t>(i)); /* fully covered */
        }
    }

    /**
     * @brief Insert `value`, replacing anything under it and merging with the
     *        previous/next range when they touch and have the same payload.
     */
    template <typename T, typename Same>
    void put_in(std::vector<T> &v, T value, Same same)
    {
        erase_in(v, { value.begin, value.end });

        const auto pos
            = std::lower_bound(v.begin(), v.end(), value.begin,
                               [](const T &r, std::uint32_t col) { return r.begin < col; });
        const auto idx = std::size_t(pos - v.begin());

        bool merged = false;

        if (idx > 0 and v[idx - 1].end == value.begin and same(v[idx - 1], value))
        {
            v[idx - 1].end = value.end;
            merged         = true;
        }

        if (idx < v.size() and v[idx].begin == value.end and same(v[idx], value))
        {
            if (merged)
            {
                v[idx - 1].end = v[idx].end;
                v.erase(v.begin() + static_cast<std::ptrdiff_t>(idx));
            }
            else
            {
                v[idx].begin = value.begin;
                merged       = true;
            }
        }

        if (!merged) v.insert(v.begin() + static_cast<std::ptrdiff_t>(idx), std::move(value));
    }
}


void row::extras::clear()
{
    uris.clear();
    underlines.clear();
}


void row::damage() noexcept
{
    for (auto &c : cells) c.attribute.set(cell::attribute::clean, false);
    this->attribute.set(attribute::clean, false);
}


void row::erase(color bg) noexcept
{
    cell blank {};
    blank.color.bg = bg;
    std::ranges::fill(cells, blank);

    if (extras != nullptr) extras->clear();

    attribute.set(attribute::wrapped, false);
    attribute.set(attribute::clean, false);
    attribute.set(attribute::prompt_row, false);

    attribute.prompt_range = { attribute::unset, attribute::unset };
}


void row::erase(range range, color bg)
{
    range.end = std::min<std::uint32_t>(range.end, std::uint32_t(cells.size()));
    if (range.begin >= range.end) return;

    cell blank {};
    blank.color.bg = bg;
    std::fill(cells.begin() + range.begin, cells.begin() + range.end, blank);

    if (extras != nullptr)
    {
        erase_in(extras->uris, range);
        erase_in(extras->underlines, range);
    }

    attribute.set(attribute::clean, false);
}


void row::resize(std::size_t columns)
{
    const auto old_columns = cells.size();
    if (columns == old_columns) return;

    if (columns > old_columns)
        cells.resize(columns); /* may throw, before anything else is modified */
    else
    {
        /* Don't leave half of a wide glyph at the new edge */
        if (cells[columns].character.is_spacer())
        {
            std::size_t c = columns;
            while (c > 0 and cells[c - 1].character.is_spacer()) c--;

            if (c > 0)
                for (std::size_t i = c - 1; i < columns; i++) cells[i].character = {};
        }

        cells.resize(columns);

        const auto cut = std::uint32_t(columns);

        if (extras != nullptr)
        {
            range r { cut, range::to_end };

            erase_in(extras->uris, r); /* the hole runs to infinity, so never splits */
            erase_in(extras->underlines, r);
        }

        if (attribute.prompt_range.begin != attribute::unset)
            attribute.prompt_range.begin = std::min(attribute.prompt_range.begin, cut);
        if (attribute.prompt_range.end != attribute::unset)
            attribute.prompt_range.end = std::min(attribute.prompt_range.end, cut);
    }

    damage();
}


void row::put_uri(range range, std::string_view uri, std::size_t id)
{
    range.end = std::min<std::uint32_t>(range.end, std::uint32_t(cells.size()));
    if (range.begin >= range.end) return;

    if (extras == nullptr) extras = std::make_unique<struct extras>();

    put_in(extras->uris,
           uri_range {
               range,
               std::string { uri },
               id,
           },
           [](const uri_range &a, const uri_range &b) { return a.id == b.id and a.uri == b.uri; });
}


void row::put_underline(range range, color color, underline_style style)
{
    range.end = std::min<std::uint32_t>(range.end, std::uint32_t(cells.size()));
    if (range.begin >= range.end) return;

    if (extras == nullptr) extras = std::make_unique<struct extras>();

    put_in(extras->underlines, underline_range { range, color, style },
           [](const underline_range &a, const underline_range &b)
           { return a.style == b.style and a.color == b.color; });
}


void row::erase_uris(range range) /* NOLINT */
{
    if (extras != nullptr)
        erase_in(extras->uris, { range.begin, std::min(range.end, range::to_end) });
}

void row::erase_underlines(range range) /* NOLINT */
{
    if (extras != nullptr)
        erase_in(extras->underlines, { range.begin, std::min(range.end, range::to_end) });
}
