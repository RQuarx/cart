#include <lyra/lyra.hpp>
#include <spdlog/spdlog.h>

#include "args.hh"
#include "metadata.hh"

using cart::args;


args::args() noexcept
{
    if (char *res = std::getenv("HOME"); res != nullptr)
        this->working_directory = res;
    else
    { /* genuinely couldn't care less... how do people not have $HOME set? */
        spdlog::warn("$HOME not set, defaulting to root.");
        this->working_directory = "/";
    }

    this->window_class = metadata::name;
}


auto args::parse(std::span<char *const> args) noexcept
    -> std::expected<std::optional<struct args>, std::string>
{
    struct args parsed;
    bool        help    = false;
    bool        version = false;

    /* clang-format off */
    auto cli = lyra::cli {}
    | lyra::help { help }
    | lyra::opt { version                                       }["-V"]["--version"  ]("Show program version.")
    | lyra::opt { parsed.working_directory, "working-directory" }["-w"]["--directory"]("Set the working directory for the terminal.")
    | lyra::opt { parsed.window_class,      "class"             }["-c"]["--class"    ]("Set the window class in wayland.");
    /* clang-format on */

    if (auto res = cli.parse({ int(args.size()), args.begin().base() }); !res)
        return std::unexpected { res.message() };

    auto print = [](auto &&text)
    {
        std::cerr << text << '\n';
        return std::nullopt;
    };

    if (help) return print(cli);
    if (version)
        return print(std::format("{} {}\nCopyright (C) 2026 Kei <RQuarx@protonmail.com>",
                                 metadata::name, metadata::version, metadata::license));

    return parsed;
}
