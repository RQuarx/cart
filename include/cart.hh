#pragma once
#include <span>

#include "config.hh"


namespace cart
{
    class cart
    {
    public:
        [[nodiscard]] static auto run(std::span<char *const> argv) noexcept -> int;

    private:
        std::shared_ptr<config> config;


        cart();
    };
}
