#include <algorithm>
#include <ranges>

#include <spdlog/spdlog.h>

#include "app/config/colors.hh"
#include "app/config/utils.hh"

using namespace cart::app::cfg;

namespace
{
    template <typename T, std::size_t N>
    constexpr auto make_filled_array(T all) noexcept -> std::array<T, N>
    {
        std::array<T, N> arr;
        arr.fill(all);
        return arr;
    }
}


auto colors::get_default() noexcept -> const colors &
{
    constexpr static colors colors {};
    return colors;
}


auto colors::parse(const toml::table &colors) noexcept -> result<void>
try
{
    for (const auto &[key, value] : colors)
    {
        if (key == "foreground" or key == "background")
        {
            mf_parse_foreground_and_background(key, value);
            continue;
        }

        if (key == "palette")
        {
            mf_parse_palette(key, value);
            continue;
        }

        if (key == "bold_as_bright")
        {
            m_bold_as_bright = utils::as<bool>(key, value);
            continue;
        }

        return error { "Unexpected node in table \"colors\": {}", key.str() }.unexpected();
    }

    return {};
}
catch (error &e)
{
    return std::move(e).unexpected();
}


void colors::mf_parse_foreground_and_background(std::string_view key, const toml::node &node)
{
    auto color = triple<std::uint32_t>::all(unset);

    for (const auto &[key_, value_] : utils::as<toml::table>(key, node))
        for (const auto &intensity : { intensity::bright, intensity::normal, intensity::dim })
            if (key_ == intensity_to_string(intensity))
                color[intensity] = utils::as<std::uint32_t>(key_, value_);

    auto member = key == "foreground" ? &colors::m_foreground : &colors::m_background;

    if (color[intensity::normal] == unset)
    {
        spdlog::warn("colors.{}.normal is not specified, using default value.", key);
        color[intensity::normal] = (get_default().*member)[intensity::normal];
    }

    if (color[intensity::bright] == unset)
    {
        spdlog::warn("colors.{}.bright is not specified, using normal value.", key);
        color[intensity::bright] = color[intensity::bright];
    }

    if (color[intensity::dim] == unset)
    {
        spdlog::warn("colors.{}.dim is not specified, using dimmed normal.", key);
        color[intensity::dim] = make_dim(color[intensity::normal]);
    }

    this->*member = color;
}


void colors::mf_parse_palette(std::string_view key, const toml::node &node)
{
    auto color = decltype(m_palette)::all(make_filled_array<std::uint32_t, 8>(unset));

    for (const auto &[intensity_str, node] : utils::as<toml::table>(key, node))
    {
        bool used = false;

        for (const auto &intensity : { intensity::bright, intensity::normal, intensity::dim })
        {
            if (intensity_str != intensity_to_string(intensity)) continue;

            auto arr = utils::as<toml::array>(intensity_str, node);

            if (arr.size() != 8)
                throw error { "Palette color for intensity {} doesn't have the correct "
                              "amount of colors (8).",
                              intensity_str };

            for (std::size_t i = 0; i < 8; i++)
            {
                auto &col   = color[intensity][i];
                auto  value = utils::as<std::int64_t>(intensity_str, *arr.get(i));

                if (value > 0xFFFFFF)
                    throw error { "Value {:x} too large to fit inside a "
                                  "24-bit RGB value.",
                                  value };
                col = value;
            }

            used = true;
        }

        if (used) continue;

        throw error { "Unexpected node in table \"colors.palette\": {}", key };
    }

    auto all_unset = [](const auto &arr) noexcept -> bool
    { return std::ranges::all_of(arr, std::bind_front(std::equal_to {}, unset)); };

    for (const auto &inst : { intensity::normal, intensity::bright })
        if (all_unset(color[inst]))
        {
            spdlog::warn("colors.palette.{} is not specified, using default values.",
                         intensity_to_string(inst));
            color[inst] = get_default().m_palette[inst];
        }

    if (all_unset(color[intensity::dim]))
    {
        spdlog::warn("colors.palette.dim is not specified, using dimmed normal values.");

        for (const auto &[val, normal] :
             std::views::zip(color[intensity::dim], m_palette[intensity::normal]))
            val = make_dim(normal);
    }

    m_palette = color;
}
