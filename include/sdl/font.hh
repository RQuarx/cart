#pragma once
#include <filesystem>
#include <string>
#include <unordered_map>

#include "sdl/object.hh"
#include "sdl/pointer.hh"


namespace cart::sdl
{
    class font;


    namespace _impl
    {
        struct font_hash
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


        inline std::unordered_map<std::filesystem::path, font, font_hash> font_library;
    }


    class font final : object<>, public sptr<TTF_Font, TTF_CloseFont>
    {
    public:
        [[nodiscard]]
        static auto get_path(const std::string &family) noexcept -> result<std::filesystem::path>;

        [[nodiscard]]
        static auto load(const std::filesystem::path &font_file, float pt) noexcept
            -> result<font>;

        constexpr font(pointer ptr) noexcept : sptr { ptr } {  }


        auto get_size() noexcept -> float;
        auto set_size(float pt) noexcept -> result<void>;
    };
}
