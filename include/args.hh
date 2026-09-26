#pragma once
#include <expected>
#include <filesystem>

#include "error.hh"


namespace cart
{
    struct args
    {
        std::filesystem::path working_directory;
        std::filesystem::path config_file;
        std::string           window_class;


        args();


        [[nodiscard]]
        static auto parse(std::span<char *const> args) noexcept
            -> std::expected<std::optional<struct args>, error>;
    };
}
