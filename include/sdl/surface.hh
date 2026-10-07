#pragma once
#include "sdl/runtime_guard.hh"
#include "sdl/types.hh"
#include "shared/traits.hh"


namespace cart::sdl
{
    class surface final : runtime_guard<>, public trait::unique_handle_of<SDL_Surface, SDL_DestroySurface>
    {
    public:
        using unique_handle_of::unique_handle_of;


        [[nodiscard]]
        constexpr auto get_size() const noexcept -> size
        { return { static_cast<float>(get()->w), static_cast<float>(get()->h) }; }
    };
}
