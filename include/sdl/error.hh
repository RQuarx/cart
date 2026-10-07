#pragma once
#include <SDL3/SDL_error.h>

#include "shared/result.hh"


namespace cart::sdl
{
    class error final : public cart::error
    {
    public:
        template <typename... Args>
        error(_impl::format_string<Args...> fmt, Args &&...args)
            : cart::error { fmt.source, "{}: {}", std::format(fmt.fmt, std::forward<Args>(args)...),
                            SDL_GetError() }
        {
        }
    };
}
