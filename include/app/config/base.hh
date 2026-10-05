#pragma once
#include <toml++/toml.hpp>

#include "shared/result.hh"


namespace cart::app::conf
{
    struct base
    {
        virtual ~base()                                                        = default;
        virtual auto parse(const toml::table &config) noexcept -> result<void> = 0;
    };
}
