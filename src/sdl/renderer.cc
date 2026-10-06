#include "sdl/renderer.hh"

using cart::sdl::renderer;


auto renderer::set_draw_color(color clear_color) noexcept -> result<>
{
    if (!SDL_SetRenderDrawColorFloat(get(), clear_color.r, clear_color.g, clear_color.b,
                                     clear_color.a))
        return error { "Failed to set render draw color: {}", SDL_GetError() }.unexpected();
    return {};
}


auto renderer::clear(color clear_color) noexcept -> result<>
{
    if (auto res = set_draw_color(clear_color); !res) return res.error().unexpected();

    if (!SDL_RenderClear(get()))
        return error { "Failed to clear window: {}", SDL_GetError() }.unexpected();
    return {};
}


auto renderer::present() noexcept -> result<>
{
    if (!SDL_RenderPresent(get()))
        return error { "Failed to present render draws: {}", SDL_GetError() }.unexpected();
    return {};
}
