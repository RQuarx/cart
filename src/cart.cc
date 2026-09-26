#include <spdlog/spdlog.h>

#include "args.hh"
#include "cart.hh"


auto cart::cart::run(std::span<char *const> argv) noexcept -> int
{
    struct args args;

    if (auto res = args::parse(argv); res.has_value())
    {
        if (!res->has_value()) return 0;
        args = std::move(**res);
    }
    else
    {
        spdlog::critical("{}", res.error().what());
        return 1;
    }

    cart c {};

    if (auto res = config::fetch(args.config_file); res.has_value())
        c.config = *std::move(res);
    else
    {
        spdlog::critical("{}", res.error().what());
        return 1;
    }

    return 0;
}


cart::cart::cart() {}