#include "app/render/renderer.hh"

using cart::app::render::renderer;


renderer::renderer(window &window) noexcept : m_renderer { window.get_renderer() } {}


auto renderer::on_frame(const config &config) noexcept -> result<action>
{
    if (auto res = m_renderer.clear(
            sdl::color::from24(config.get_colors().get_background(cfg::colors::intensity::normal)));
        !res)
        return res.error().unexpected();
    if (auto res = m_renderer.present(); !res) return res.error().unexpected();
    return action::continue_process;
}
