#pragma once
#include <span>

#include "app/args.hh"
#include "app/config.hh"
#include "app/render/renderer.hh"
#include "app/window.hh"


namespace cart::app
{
    class cart
    {
    public:
        [[nodiscard]] static auto run(std::span<char *const> argv) noexcept -> int;

    private:
        args   m_args;
        window m_window;

        render::renderer m_renderer;

        std::shared_ptr<config> m_config;


        [[nodiscard]] auto on_frame() noexcept -> result<action>;


        cart(args &&args, std::shared_ptr<config> &&config, window &&window) noexcept;


        void mf_on_window_resized(int new_width, int new_height) noexcept;
    };
}
