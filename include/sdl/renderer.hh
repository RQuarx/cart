#pragma once
#include <SDL3/SDL_render.h>

#include "sdl/runtime_guard.hh"
#include "sdl/surface.hh"
#include "sdl/texture.hh"
#include "sdl/types.hh"
#include "shared/traits.hh"


namespace cart::sdl
{
    class renderer final : runtime_guard<>,
                           public trait::shared_handle_of<SDL_Renderer, SDL_DestroyRenderer>
    {
    public:
        using shared_handle_of::shared_handle_of;


        auto set_draw_color(color color) noexcept -> result<>;
        auto clear(color clear_color) noexcept -> result<>;
        auto present() noexcept -> result<>;

        auto render_texture(texture &texture, rect dst) noexcept -> result<>;
        auto render_surface(surface &surface, rect dst) noexcept -> result<>;
    };
}
