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
        return _impl::font_library.emplace(font_file, f).first->second;

    return shared::error { "Failed to open font file \"{}\": {}", font_file.c_str(),
                           SDL_GetError() }
        .unexpected();
}


auto font::get_size() noexcept -> float { return TTF_GetFontSize(get()); }
auto font::set_size(float pt) noexcept -> result<void>
{
    if (!TTF_SetFontSize(get(), pt))
        return shared::error { "Failed to set font size: {}", SDL_GetError() }.unexpected();
    return {};
}
