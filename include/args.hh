#pragma once
#include <expected>
#include <filesystem>


namespace cart
{
    struct args
    {
        std::filesystem::path working_directory;
        std::string           window_class;


        args() noexcept;


        [[nodiscard]]
        static auto parse(std::span<char *const> args) noexcept
            -> std::expected<std::optional<struct args>, std::string>;
    };
}
