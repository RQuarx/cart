#pragma once
#include <span>

#include "app/config.hh"


namespace cart::app
{
    class cart
    {
    public:
        [[nodiscard]] static auto run(std::span<char *const> argv) noexcept -> int;

    private:
        std::shared_ptr<app::config> m_config;


        cart();
    };
}
