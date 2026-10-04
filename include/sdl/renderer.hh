#pragma once
#include <SDL3/SDL_render.h>

#include "sdl/object.hh"
#include "sdl/pointer.hh"


namespace cart::sdl
{
    class renderer final : object<>, public sptr<SDL_Renderer, SDL_DestroyRenderer>
    {
    public:
        renderer() = default;
        constexpr renderer(SDL_Renderer *renderer) noexcept { reset(renderer); }

    private:
    };
}
