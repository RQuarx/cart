#include "app/render/renderer.hh"

using cart::app::render::renderer;


renderer::renderer(window &window) noexcept : m_renderer { window.get_renderer() } {}


auto renderer::on_frame() noexcept -> result<action> {}
