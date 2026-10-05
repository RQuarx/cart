#include <spdlog/spdlog.h>

#include "app/window.hh"
#include "sdl/event.hh"

using cart::app::window;


auto window::create(const std::shared_ptr<config> &config) noexcept -> result<window>
{
    if (auto res
        = sdl::window::create(config->get_window().get_title().data(), 800, 600); /* NOLINT */
        res.has_value())
        return window { std::move(res->first), std::move(res->second) };
    else /* NOLINT */
        return res.error().unexpected();
}


auto window::on_frame() noexcept -> result<action>
{
    return sdl::event_pump::drain(
        [&](const sdl::event &event) -> result<action>
        {
            if (event.is_window_event()) return mf_handle_window_event(event);

            return action::continue_process;
        });
}


auto window::mf_handle_window_event(const sdl::event &event) noexcept -> result<action>
{
    switch (event.type())
    {
    case SDL_EVENT_WINDOW_RESIZED:
        m_on_resize_callback(event.as_window()->data1, event.as_window()->data2);
        break;

    default: break;
    }

    return action::continue_process;
}


auto window::get_renderer() noexcept -> sdl::renderer & { return m_renderer; }
