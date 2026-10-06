#pragma once
#include <SDL3/SDL_render.h>

#include "sdl/color.hh"
#include "sdl/object.hh"
#include "sdl/pointer.hh"


namespace cart::sdl
{
    class renderer final : object<>, public sptr<SDL_Renderer, SDL_DestroyRenderer>
    {
    public:
        renderer() = default;
        constexpr renderer(pointer renderer) noexcept : sptr { renderer } {}


        auto set_draw_color(color color) noexcept -> result<>;
        auto clear(color clear_color) noexcept -> result<>;
        auto present() noexcept -> result<>;
    };
}
