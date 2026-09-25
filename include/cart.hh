#pragma once
#include <span>


namespace cart
{
    class cart
    {
    public:
        [[nodiscard]] static auto run(std::span<char *const> argv) noexcept -> int;

    private:
        cart();
    };
}