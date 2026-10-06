#include <algorithm>

#include <fontconfig/fontconfig.h>

#include "sdl/font.hh"

using cart::sdl::font;

namespace
{
    [[nodiscard]]
    auto parse_font_style(std::string_view text) noexcept -> std::optional<TTF_FontStyleFlags>
    {
        constexpr std::string_view separators = " \t|,+";
        TTF_FontStyleFlags         flags      = TTF_STYLE_NORMAL;

        while (true)
        {
            const auto start = text.find_first_not_of(separators);
            if (start == std::string_view::npos) break;
            text.remove_prefix(start);

            const auto             end        = text.find_first_of(separators);
            const std::string_view token_view = text.substr(0, end);
            text.remove_prefix(end == std::string_view::npos ? text.size() : end);

            std::string token { token_view };
            std::ranges::transform(token, token.begin(), [](unsigned char c)
                                   { return static_cast<char>(std::tolower(c)); });

            if (token == "normal" or token == "regular")
                ;
            else if (token == "bold")
                flags |= TTF_STYLE_BOLD;
            else if (token == "italic")
                flags |= TTF_STYLE_ITALIC;
            else if (token == "underline")
                flags |= TTF_STYLE_UNDERLINE;
            else if (token == "strikethrough")
                flags |= TTF_STYLE_STRIKETHROUGH;
            else
                return std::nullopt;
        }

        return flags;
    }
}


auto font::get_path(std::string_view family, std::string_view style) noexcept
    -> result<std::filesystem::path>
{
    if (std::filesystem::exists(family) and std::filesystem::is_regular_file(family)) return family;

    FcConfig *cfg = FcInitLoadConfigAndFonts();
    if (cfg == nullptr) return error { "Failed to initialize fontconfig" }.unexpected();

    FcPattern *pattern = FcPatternCreate();
    FcPatternAddString(pattern, FC_FAMILY, reinterpret_cast<const FcChar8 *>(family.data()));
    FcPatternAddString(pattern, FC_STYLE, reinterpret_cast<const FcChar8 *>(style.data()));

    std::filesystem::path path;
    FcResult              res;
    if (FcPattern *matched = FcFontMatch(cfg, pattern, &res); matched != nullptr)
    {
        if (FcChar8 *file_path = nullptr;
            FcPatternGetString(matched, FC_FILE, 0, &file_path) == FcResultMatch)
            path = reinterpret_cast<char *>(file_path);
        else
            return error { "Font family \"{}\" not found.", family }.unexpected();
        FcPatternDestroy(matched);
    }

    FcPatternDestroy(pattern);
    FcFini();

    return path;
}


auto font::load(const std::filesystem::path &font_file, float pt, std::string_view style) noexcept
    -> result<font>
{
    if (auto it = _impl::font_library.find(font_file); it != _impl::font_library.end())
    {
        auto &font = it->second;
        if (font.get_size() != pt)
            if (auto res = font.set_size(pt); !res) return res.error().unexpected();
        return font;
    }

    if (TTF_Font *f = TTF_OpenFont(font_file.c_str(), pt); f != nullptr)
    {
        TTF_SetFontKerning(f, false);
        auto &font = _impl::font_library.emplace(font_file, f).first->second;
        if (auto res = font.set_style(style); !res) return res.error().unexpected();
        return font;
    }

    return error { "Failed to open font file \"{}\": {}", font_file.c_str(), SDL_GetError() }
        .unexpected();
}


auto font::get_size() noexcept -> float { return TTF_GetFontSize(get()); }
auto font::set_size(float pt) noexcept -> result<>
{
    if (!TTF_SetFontSize(get(), pt))
        return error { "Failed to set font size: {}", SDL_GetError() }.unexpected();
    return {};
}

auto font::set_size(float pt, int horizontal_dpi, int vertical_dpi) noexcept -> result<>
{
    if (!TTF_SetFontSizeDPI(get(), pt, horizontal_dpi, vertical_dpi))
        return error { "Failed to set font size: {}", SDL_GetError() }.unexpected();
    return {};
}


auto font::get_glyph_metrics(std::uint32_t character) -> metrics
{
    metrics m;
    if (!TTF_GetGlyphMetrics(get(), character, &m.x.min, &m.x.max, &m.y.min, &m.y.max, &m.advance))
        throw error { "Failed to get glyph metrics for '{}': {}", character, SDL_GetError() };
    return m;
}


auto font::get_ascent() noexcept -> int { return TTF_GetFontAscent(get()); }
auto font::get_descent() noexcept -> int { return TTF_GetFontDescent(get()); }
auto font::get_height() noexcept -> int { return TTF_GetFontHeight(get()); }
auto font::get_line_skip() noexcept -> int { return TTF_GetFontLineSkip(get()); }
auto font::get_string_size(const std::string &string) -> std::pair<int, int>
{
    std::pair<int, int> res;

    if (!TTF_GetStringSize(get(), string.c_str(), string.size(), &res.first, &res.second))
        throw error { "Failed to get the size of string \"{}\": {}", string, SDL_GetError() };
    return res;
}


auto font::is_monospace() noexcept -> bool { return TTF_FontIsFixedWidth(get()); }


auto font::set_style(std::string_view style_string) noexcept -> result<>
{
    if (auto res = parse_font_style(style_string); res.has_value())
    {
        TTF_SetFontStyle(get(), *res);
        return {};
    }

    return error { "Style string (\"{}\") contains an invalid style.", style_string }.unexpected();
}
