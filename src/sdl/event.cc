#include "sdl/error.hh"
#include "sdl/event.hh"

using cart::sdl::event_pump;


auto event_pump::poll() noexcept -> std::optional<event>
{
    SDL_Event raw;
    if (SDL_PollEvent(&raw)) return event { raw };
    return std::nullopt;
}


auto event_pump::wait() noexcept -> result<event>
{
    SDL_Event raw;
    if (!SDL_WaitEvent(&raw)) return sdl::error { "Failed to wait for an event" }.unexpected();
    return event { raw };
}


auto event_pump::wait_for(std::chrono::milliseconds timeout) noexcept -> std::optional<event>
{
    SDL_Event raw;
    if (SDL_WaitEventTimeout(&raw, static_cast<Sint32>(timeout.count()))) return event { raw };
    return std::nullopt;
}


auto event_pump::push(const event &e) noexcept -> result<>
{
    SDL_Event raw = e.get();
    if (!SDL_PushEvent(&raw)) return sdl::error { "Failed to push event" }.unexpected();
    return {};
}


auto event_pump::push_quit() noexcept -> result<>
{
    SDL_Event raw {};
    raw.type = SDL_EVENT_QUIT;
    if (!SDL_PushEvent(&raw)) return sdl::error { "Failed to push quit event" }.unexpected();
    return {};
}
