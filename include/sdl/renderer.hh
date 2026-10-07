#pragma once
#include <SDL3/SDL_render.h>

#include "sdl/object.hh"
#include "sdl/surface.hh"
#include "sdl/texture.hh"
#include "sdl/types.hh"
#include "shared/traits.hh"


namespace cart::sdl
{
    class renderer final : object<>, public trait::sptr<SDL_Renderer, SDL_DestroyRenderer>
    {
    public:
        using sptr::sptr;


        auto set_draw_color(color color) noexcept -> result<>;
        auto clear(color clear_color) noexcept -> result<>;
        auto present() noexcept -> result<>;

        auto render_texture(texture &texture, rect dst) noexcept -> result<>;
        auto render_surface(surface &surface, rect dst) noexcept -> result<>;
    };
}
