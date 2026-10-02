#include <spdlog/spdlog.h>

#include "app/args.hh"
#include "app/cart.hh"


auto cart::app::cart::run(std::span<char *const> argv) noexcept -> int
{
    struct args args;

    if (auto res = args::parse(argv); res.has_value())
    {
        if (!res->has_value()) return 0;
        args = std::move(**res);
    }
    else
    {
        spdlog::critical("{}", res.error().format());
        return 1;
    }

    cart c {};

    if (auto res = config::fetch(args.config_file); res.has_value())
        c.m_config = *std::move(res);
    else
    {
        spdlog::critical("{}", res.error().format());
        return 1;
    }

    return 0;
}


cart::app::cart::cart() {}