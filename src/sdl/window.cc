#include "sdl/window.hh"

using cart::sdl::window;


auto window::create(const char *title, int w, int h) noexcept -> result<std::pair<window, renderer>>
try
{
    pointer           wind;
    renderer::pointer rend;

    if (!SDL_CreateWindowAndRenderer(
            title, w, h, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY, &wind, &rend))
        return error { "Failed to create window and renderer: {}", SDL_GetError() }.unexpected();

    SDL_SetRenderVSync(rend, 1);

    return std::pair { window { wind }, renderer { rend } };
}
catch (error &e)
{
    return std::move(e).unexpected();
}


auto window::get_id() const noexcept -> std::uint64_t { return SDL_GetWindowID(get()); }


auto window::get_size() const noexcept -> result<size>
{
    int w;
    int h;

    if (!SDL_GetWindowSize(get(), &w, &h))
        return error { "Failed to get window size: {}", SDL_GetError() }.unexpected();
    return size { static_cast<float>(w), static_cast<float>(h) };
}


auto window::get_size_in_pixels() const noexcept -> result<size>
{
    int w;
    int h;

    if (!SDL_GetWindowSizeInPixels(get(), &w, &h))
        return error { "Failed to get window size in pixels: {}", SDL_GetError() }.unexpected();
    return size { static_cast<float>(w), static_cast<float>(h) };
}


auto window::get_display_scale() const noexcept -> float
{ return SDL_GetWindowDisplayScale(get()); }


auto window::get_pixel_density() const noexcept -> float
{ return SDL_GetWindowPixelDensity(get()); }


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
