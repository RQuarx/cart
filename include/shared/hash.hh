#pragma once
#include <filesystem>


namespace cart
{
    struct heterogeneous_hash
    {
        using is_transparent = void;

        auto operator()(const std::filesystem::path &p) const noexcept -> std::size_t
        { return std::hash<std::filesystem::path> {}(p); }

        auto operator()(std::string_view sv) const noexcept -> std::size_t
        { return std::hash<std::string_view> {}(sv); }

        auto operator()(const std::string &str) const noexcept -> std::size_t
        { return std::hash<std::string> {}(str); }

        auto operator()(const char *ptr) const noexcept -> std::size_t
        { return std::hash<std::string_view> {}(ptr); }
    };
}
