#pragma once
#include <filesystem>
#include <string>
#include <unordered_map>

#include "sdl/object.hh"
#include "sdl/pointer.hh"


namespace cart::sdl
{
    class font final : object<>, public sptr<TTF_Font, TTF_CloseFont>
    {
    public:
        struct metrics
        {
            struct
            {
                int min;
                int max;
            } x;

            struct
            {
                int min;
                int max;
            } y;

            int advance;
        };


        [[nodiscard]]
        static auto get_path(const std::string &family) noexcept -> result<std::filesystem::path>;

        [[nodiscard]]
        static auto load(const std::filesystem::path &font_file, float pt) noexcept -> result<font>;

        constexpr font(pointer ptr) noexcept : sptr { ptr } {}


        [[nodiscard]] auto get_size() noexcept -> float;
        auto               set_size(float pt) noexcept -> result<>;
        auto set_size(float pt, int horizontal_dpi, int vertical_dpi) noexcept -> result<>;

        [[nodiscard]] auto get_glyph_metrics(std::uint32_t character) -> metrics;
        [[nodiscard]] auto get_ascent() noexcept -> int;
        [[nodiscard]] auto get_descent() noexcept -> int;
        [[nodiscard]] auto get_height() noexcept -> int;
        [[nodiscard]] auto get_line_skip() noexcept -> int;

        /** @return A pair containing the width, and height. */
        [[nodiscard]] auto get_string_size(const std::string &string) -> std::pair<int, int>;
        [[nodiscard]] auto is_monospace() noexcept -> bool;
    };


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
}
