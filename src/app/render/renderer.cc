#include <spdlog/spdlog.h>

#include "app/render/renderer.hh"

using cart::app::render::renderer;


auto renderer::create(window &window, const config &config) noexcept -> result<renderer>

{
    renderer r;
    r.m_renderer = window.get_renderer();

    if (auto res = r.mf_load_fonts(config.get_font()); !res) return res.error().unexpected();
    return r;
}


auto renderer::on_frame(const config &config) noexcept -> result<action>
{
    if (auto res = m_renderer.clear(
            sdl::color::from24(config.get_colors().get_background(cfg::colors::intensity::normal)));
        !res)
        return res.error().unexpected();

    if (auto res = m_renderer.present(); !res) return res.error().unexpected();
    return action::continue_process;
}


auto renderer::on_config_changed(const config &config) noexcept -> result<>
{ return mf_load_fonts(config.get_font()); }


auto renderer::mf_load_fonts(const cfg::font &font_config) noexcept -> result<>
{
    const float size = font_config.get_size();

    static constexpr std::array methods {
        std::pair { font_style::normal,      &cfg::font::get_normal      },
        std::pair { font_style::bold,        &cfg::font::get_bold        },
        std::pair { font_style::italic,      &cfg::font::get_italic      },
        std::pair { font_style::bold_italic, &cfg::font::get_bold_italic },
    };

    for (const auto &[style, method] : methods)
    {
        std::string_view family = (font_config.*method)(cfg::font::family);
        std::string_view fstyle = (font_config.*method)(cfg::font::style);

        if (auto res = sdl::font::open(family, fstyle, size); res.has_value())
            m_fonts[std::to_underlying(style)] = std::move(*res);
        else
            return res.error().unexpected();

        spdlog::info(R"(Loaded font "{}" with a style of "{}".)", family, fstyle);
    }

    return {};
}
