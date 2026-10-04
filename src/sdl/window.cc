#include "sdl/window.hh"

using cart::sdl::window;


auto window::create(const char *title, int w, int h) noexcept -> result<window>
{
    pointer           wind;
    renderer::pointer renderer;

    if (!SDL_CreateWindowAndRenderer(
            title, w, h, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY, &wind, &renderer))
        return shared::error { "Failed to create window and renderer: {}", SDL_GetError() }
            .unexpected();

    return window { wind, renderer };
}


auto window::get_id() noexcept -> std::uint64_t { return SDL_GetWindowID(get()); }


auto window::get_size()  -> std::pair<int, int>
{
    std::pair<int, int> size;
    if (!SDL_GetWindowSize(get(), &size.first, &size.second))
        throw shared::error { "Failed to get window size: {}", SDL_GetError() };
    return size;
}


auto window::get_size_in_pixels()  -> std::pair<int, int>
{
    std::pair<int, int> size;
    if (!SDL_GetWindowSizeInPixels(get(), &size.first, &size.second))
        throw shared::error { "Failed to get window size in pixels: {}", SDL_GetError() };
    return size;
}


