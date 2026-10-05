#include "sdl/window.hh"

using cart::sdl::window;


auto window::create(const char *title, int w, int h) noexcept -> result<std::pair<window, renderer>>
{
    pointer           wind;
    renderer::pointer rend;

    if (!SDL_CreateWindowAndRenderer(
            title, w, h, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY, &wind, &rend))
        return error { "Failed to create window and renderer: {}", SDL_GetError() }.unexpected();

    SDL_SetRenderVSync(rend, 1);

    return std::pair { window { wind }, renderer { rend } };
}


auto window::get_id() noexcept -> std::uint64_t { return SDL_GetWindowID(get()); }


auto window::get_size() -> std::pair<int, int>
{
    std::pair<int, int> size;
    if (!SDL_GetWindowSize(get(), &size.first, &size.second))
        throw error { "Failed to get window size: {}", SDL_GetError() };
    return size;
}


auto window::get_size_in_pixels() -> std::pair<int, int>
{
    std::pair<int, int> size;
    if (!SDL_GetWindowSizeInPixels(get(), &size.first, &size.second))
        throw error { "Failed to get window size in pixels: {}", SDL_GetError() };
    return size;
}


auto window::get_display_scale() noexcept -> float { return SDL_GetWindowDisplayScale(get()); }
auto window::get_pixel_density() noexcept -> float { return SDL_GetWindowPixelDensity(get()); }

auto window::start_text_input() noexcept -> result<>
{
    if (!SDL_StartTextInput(get()))
        return error { "Failed to start text input: {}", SDL_GetError() }.unexpected();
    return {};
}

auto window::stop_text_input() noexcept -> result<>
{
    if (!SDL_StopTextInput(get()))
        return error { "Failed to stop text input: {}", SDL_GetError() }.unexpected();
    return {};
}
