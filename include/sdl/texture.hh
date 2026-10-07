#pragma once
#include "sdl/runtime_guard.hh"
#include "shared/traits.hh"


namespace cart::sdl
{
    class texture final : runtime_guard<>, public trait::unique_handle_of<SDL_Texture, SDL_DestroyTexture>
    {
    public:
        using unique_handle_of::unique_handle_of;
    };
}
