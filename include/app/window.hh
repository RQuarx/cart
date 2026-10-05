#pragma once
#include <memory>

#include "app/config.hh"
#include "sdl/event.hh"
#include "sdl/renderer.hh"
#include "sdl/window.hh"
#include "shared/callback.hh"
#include "shared/result.hh"


namespace cart::app
{
    class window
    {
    public:
        using resize_callback = callback<void(int, int)>;


        [[nodiscard]]
        static auto create(const std::shared_ptr<config> &config) noexcept -> result<window>;

        auto on_frame() noexcept -> result<action>;

        [[nodiscard]]
        auto get_window_size() noexcept -> std::pair<int, int>;

        [[nodiscard]]
        auto get_renderer() noexcept -> sdl::renderer &;

        template <typename Self>
        void set_on_resize_callback(Self &self, void (Self::*fn)(int, int))
        { m_on_resize_callback.set(self, fn); }


    private:
        sdl::window   m_window;
        sdl::renderer m_renderer;

        resize_callback m_on_resize_callback;


        constexpr window(sdl::window &&window, sdl::renderer &&renderer) noexcept
            : m_window { std::move(window) }, m_renderer { std::move(renderer) }
        {
        }


        [[nodiscard]]
        auto mf_handle_window_event(const sdl::event &event) noexcept -> result<action>;
    };
}
