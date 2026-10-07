#pragma once
#include "sdl/object.hh"
#include "shared/traits.hh"


namespace cart::sdl
{
    class texture final : object<>, public trait::uptr<SDL_Texture, SDL_DestroyTexture>
    {
    public:
        using uptr::uptr;
    };
}
