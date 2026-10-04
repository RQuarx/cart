#include <fontconfig/fontconfig.h>

#include "sdl/font.hh"

using cart::sdl::font;


auto font::get_path(const std::string &family) noexcept -> result<std::filesystem::path>
{
    if (std::filesystem::exists(family) and std::filesystem::is_regular_file(family)) return family;

    FcConfig *cfg = FcInitLoadConfigAndFonts();
    if (cfg == nullptr) return shared::error { "Failed to initialize fontconfig" }.unexpected();

    FcPattern *pattern = FcPatternCreate();
    FcPatternAddString(pattern, FC_FAMILY, reinterpret_cast<const FcChar8 *>(family.c_str()));

    std::filesystem::path path;
    FcResult              res;
    if (FcPattern *matched = FcFontMatch(cfg, pattern, &res); matched != nullptr)
    {
        if (FcChar8 *file_path = nullptr;
            FcPatternGetString(matched, FC_FILE, 0, &file_path) == FcResultMatch)
            path = reinterpret_cast<char *>(file_path);
        else
            return shared::error { "Font family \"{}\" not found.", family }.unexpected();
        FcPatternDestroy(matched);
    }

    FcPatternDestroy(pattern);
    FcFini();

    return path;
}


auto font::load(const std::filesystem::path &font_file, float pt) noexcept -> result<font>
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
        return _impl::font_library.emplace(font_file, f).first->second;
    }

    return shared::error { "Failed to open font file \"{}\": {}", font_file.c_str(),
                           SDL_GetError() }
        .unexpected();
}


auto font::get_size() noexcept -> float { return TTF_GetFontSize(get()); }
auto font::set_size(float pt) noexcept -> result<>
{
    if (!TTF_SetFontSize(get(), pt))
        return shared::error { "Failed to set font size: {}", SDL_GetError() }.unexpected();
    return {};
}

auto font::set_size(float pt, int horizontal_dpi, int vertical_dpi) noexcept -> result<>
{
    if (!TTF_SetFontSizeDPI(get(), pt, horizontal_dpi, vertical_dpi))
        return shared::error { "Failed to set font size: {}", SDL_GetError() }.unexpected();
    return {};
}


auto font::get_glyph_metrics(std::uint32_t character) -> metrics
{
    metrics m;
    if (!TTF_GetGlyphMetrics(get(), character, &m.x.min, &m.x.max, &m.y.min, &m.y.max, &m.advance))
        throw shared::error { "Failed to get glyph metrics for '{}': {}", character,
                              SDL_GetError() };
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
        throw shared::error { "Failed to get the size of string \"{}\": {}", string,
                              SDL_GetError() };
    return res;
}


auto font::is_monospace() noexcept -> bool { return TTF_FontIsFixedWidth(get()); }
