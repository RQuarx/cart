#pragma once
#include <SDL3/SDL_init.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "sdl/error.hh"
#include "shared/result.hh"
#include "shared/traits.hh"


namespace cart::sdl
{
    template <SDL_InitFlags F>
    struct runtime_policy
    {
        static auto init() noexcept -> result<>
        {
            if (!SDL_Init(F))
                return sdl::error { "Failed to initialize SDL with flags {}", F }.unexpected();
            if (!TTF_Init()) return sdl::error { "Failed to initialize SDL_TTF" }.unexpected();
            return {};
        }

        static void deinit()
        {
            TTF_Quit();
            SDL_Quit();
        }
    };


    template <SDL_InitFlags F = SDL_INIT_VIDEO>
    using runtime_guard = trait::runtime_guard<runtime_policy<F>>;
}
