#include "sdl/error.hh"
#include "sdl/renderer.hh"

using cart::sdl::renderer;


auto renderer::set_draw_color(color clear_color) noexcept -> result<>
{
    if (!SDL_SetRenderDrawColorFloat(get(), clear_color.r, clear_color.g, clear_color.b,
                                     clear_color.a))
        return sdl::error { "Failed to set render draw color" }.unexpected();
    return {};
}


auto renderer::clear(color clear_color) noexcept -> result<>
{
    if (auto res = set_draw_color(clear_color); !res) return res.error().unexpected();

    if (!SDL_RenderClear(get())) return sdl::error { "Failed to clear window" }.unexpected();
    return {};
}


auto renderer::present() noexcept -> result<>
{
    if (!SDL_RenderPresent(get()))
        return sdl::error { "Failed to present render draws" }.unexpected();
    return {};
}


auto renderer::render_texture(texture &texture, rect dst) noexcept -> result<>
{
    if (!SDL_RenderTexture(get(), texture.get(), nullptr, &dst.to_frect()))
        return sdl::error { "Failed to render texture" }.unexpected();
    return {};
}


auto renderer::create_texture(surface &surface) noexcept -> result<texture>
{
    if (texture::pointer ptr = SDL_CreateTextureFromSurface(get(), surface.get()); ptr != nullptr)
        return texture { ptr };
    return sdl::error { "Failed to create texture from surface" }.unexpected();
}
