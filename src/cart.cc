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
        spdlog::critical("Failed to parse command line arguments: {}", res.error());
        return 1;
    }

    return 0;
}
