#pragma once
#include "sdl/object.hh"
#include "sdl/types.hh"
#include "shared/traits.hh"


namespace cart::sdl
{
    class surface final : object<>, public trait::uptr<SDL_Surface, SDL_DestroySurface>
    {
    public:
        using uptr::uptr;


        [[nodiscard]]
        constexpr auto get_size() const noexcept -> size
        { return { static_cast<float>(get()->w), static_cast<float>(get()->h) }; }
    };
}
