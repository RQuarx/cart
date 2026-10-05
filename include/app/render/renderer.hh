#pragma once
#include "app/window.hh"
#include "sdl/renderer.hh"


namespace cart::app::render
{
    class renderer
    {
    public:
        renderer(window &window) noexcept;


        [[nodiscard]]
        auto on_frame(const term::theme &theme) noexcept -> result<action>;

    private:
        sdl::renderer m_renderer;
    };
}
