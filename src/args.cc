#include <lyra/lyra.hpp>
#include <spdlog/spdlog.h>

#include "args.hh"
#include "error.hh"
#include "metadata.hh"

using cart::args;


args::args()
{
    namespace fs = std::filesystem;
    fs::path home;

    if (char *res = std::getenv("HOME"); res != nullptr)
    {
        home                    = res;
        this->working_directory = home;
    }
    else
        throw error { "$HOME is not set, aborting." };

    if (char *res = std::getenv("XDG_CONFIG_HOME"); res != nullptr)
        this->config_file = std::filesystem::path { res } / metadata::name / "config.toml";
    else
        this->config_file = home / ".config" / metadata::name / "config.toml";

    this->window_class = metadata::name;
}


auto args::parse(std::span<char *const> args) noexcept
    -> std::expected<std::optional<struct args>, error>
try
{
    struct args parsed;

    bool help    = false;
    bool version = false;

    /* clang-format off */
    auto cli = lyra::cli {}
    | lyra::help { help }
    | lyra::opt { version                               }["-V"]["--version"  ]("Show program version.")
    | lyra::opt { parsed.working_directory, "directory" }["-w"]["--directory"]("Set the working directory for the terminal.")
    | lyra::opt { parsed.window_class,      "class"     }["-c"]["--class"    ]("Set the window class in wayland.")
    | lyra::opt { parsed.config_file,       "file"      }["-C"]["--config"   ]("Set a custom config file.");
    /* clang-format on */

    if (auto res = cli.parse({ int(args.size()), args.begin().base() }); !res)
        return error { "Failed to parse command-line arguments: {}", res.message() }.unexpected();

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
catch (error &e)
{
    return e.unexpected();
}
