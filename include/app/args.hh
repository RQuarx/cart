#pragma once
#include <filesystem>

#include "shared/result.hh"


namespace cart::app
{
    struct args
    {
        std::filesystem::path working_directory;
        std::filesystem::path config_file;
        std::string           window_class;


        args();


        [[nodiscard]]
        static auto parse(std::span<char *const> args) noexcept -> result<std::optional<app::args>>;
    };
}
