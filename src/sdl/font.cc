#include <algorithm>
#include <utility>

#include <fontconfig/fontconfig.h>

#include "sdl/error.hh"
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
    if (cfg == nullptr) return cart::error { "Failed to initialize fontconfig" }.unexpected();

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
            return cart::error { "Font family \"{}\" not found.", family }.unexpected();
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

    return sdl::error { "Failed to open font file \"{}\"", font_file.c_str() }.unexpected();
}


auto font::get_size() noexcept -> float { return TTF_GetFontSize(get()); }

auto font::set_size(float pt) noexcept -> result<>
{
    if (!TTF_SetFontSize(get(), pt)) return sdl::error { "Failed to set font size" }.unexpected();
    return {};
}

auto font::set_size(float pt, int horizontal_dpi, int vertical_dpi) noexcept -> result<>
{
    if (!TTF_SetFontSizeDPI(get(), pt, horizontal_dpi, vertical_dpi))
        return sdl::error { "Failed to set font size" }.unexpected();
    return {};
}


auto font::get_glyph_metrics(std::uint32_t character) -> metrics
{
    metrics m;
    if (!TTF_GetGlyphMetrics(get(), character, &m.x.min, &m.x.max, &m.y.min, &m.y.max, &m.advance))
        throw sdl::error { "Failed to get glyph metrics for '{}'", character };
    return m;
}


auto font::get_ascent() noexcept -> int { return TTF_GetFontAscent(get()); }
auto font::get_descent() noexcept -> int { return TTF_GetFontDescent(get()); }
auto font::get_height() noexcept -> int { return TTF_GetFontHeight(get()); }
auto font::get_line_skip() noexcept -> int { return TTF_GetFontLineSkip(get()); }
auto font::get_string_size(const std::string &string) -> size
{
    int w = 0;
    int h = 0;

    if (!TTF_GetStringSize(get(), string.c_str(), string.size(), &w, &h))
        throw sdl::error { "Failed to get the size of string \"{}\"", string };
    return { static_cast<float>(w), static_cast<float>(h) };
}


auto font::is_monospace() noexcept -> bool { return TTF_FontIsFixedWidth(get()); }


auto font::set_style(std::string_view style_string) noexcept -> result<>
{
    if (auto res = parse_font_style(style_string); res.has_value())
    {
        TTF_SetFontStyle(get(), *res);
        return {};
    }

    return cart::error { "Style string (\"{}\") contains an invalid style.", style_string }
        .unexpected();
}


auto font::render_glyph(char32_t glyph, color fg, color bg, glyph_quality quality) noexcept
    -> result<surface>
{
    switch (quality)
    {
    case glyph_quality::shaded:
        {
            surface::pointer res
                = TTF_RenderGlyph_Shaded(get(), glyph, fg.to_color(), bg.to_color());

            if (res == nullptr) return sdl::error { "Failed to render shaded glyph" }.unexpected();
            return surface { res };
        }

    case glyph_quality::solid:
        {
            surface::pointer res = TTF_RenderGlyph_Solid(get(), glyph, fg.to_color());

            if (res == nullptr) return sdl::error { "Failed to render solid glyph" }.unexpected();
            return surface { res };
        }

    case glyph_quality::blended:
        {
            surface::pointer res = TTF_RenderGlyph_Blended(get(), glyph, fg.to_color());

            if (res == nullptr) return sdl::error { "Failed to render blended glyph" }.unexpected();
            return surface { res };
        }

    case glyph_quality::lcd:
        {
            surface::pointer res = TTF_RenderGlyph_LCD(get(), glyph, fg.to_color(), bg.to_color());

            if (res == nullptr) return sdl::error { "Failed to render LCD glyph" }.unexpected();
            return surface { res };
        }
    }

    std::unreachable();
}
