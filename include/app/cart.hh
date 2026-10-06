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
        std::shared_ptr<config> m_config;

        args   m_args;
        window m_window;

        render::renderer m_renderer;

        std::mutex  m_config_mtx;
        std::size_t m_config_changed_amount = 0;


        [[nodiscard]] auto on_frame() noexcept -> result<action>;


        cart(args                    &&args,
             std::shared_ptr<config> &&config,
             window                  &&window,
             render::renderer        &&renderer) noexcept;


        void mf_on_window_resized(int new_width, int new_height) noexcept;
        void mf_on_config_changed_worker_thread() noexcept;
        auto mf_on_config_changed_main_thread() noexcept -> result<>;
    };
}
