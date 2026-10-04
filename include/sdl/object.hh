#pragma once
#include <atomic>

#include <SDL3/SDL_init.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "shared/result.hh"


namespace cart::sdl
{
    namespace _impl { inline std::atomic<std::size_t> object_count = 0; };


    /** @brief A trait whose sole purpose is to check if SDL is intialized
     *         at an object construction. And if it is not initialized, initialize it.
     *
     * @throw The constructor might throw a @ref shared::error object if the initalization failed.
     */
    template <SDL_InitFlags F = SDL_INIT_VIDEO>
    class object
    {
    protected:
        object()
        {
            if (_impl::object_count.fetch_add(1) == 0)
            {
                if (!SDL_Init(F))
                    throw shared::error { "Failed to initialize SDL with flags {}: {}", F,
                                          SDL_GetError() };
                if (!TTF_Init())
                    throw shared::error { "Failed to initialize SDL_TTF: {}", F, SDL_GetError() };
            }
        }

        ~object()
        {
            if (_impl::object_count.fetch_sub(1) == 1)
            {
                TTF_Quit();
                SDL_Quit();
            };
        }
    };
}
