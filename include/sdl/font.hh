#pragma once
#include <filesystem>
#include <string>
#include <unordered_map>

#include "sdl/runtime_guard.hh"
#include "sdl/surface.hh"
#include "sdl/types.hh"
#include "shared/hash.hh"
#include "shared/traits.hh"


namespace cart::sdl
{
    class font final : runtime_guard<>, public trait::shared_handle_of<TTF_Font, TTF_CloseFont>
    {
    public:
        using shared_handle_of::shared_handle_of;


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


        enum class glyph_quality : std::uint8_t
        {
            shaded,
            blended,
            solid,
            lcd,
        };


        [[nodiscard]]
        static auto get_path(std::string_view family, std::string_view style) noexcept
            -> result<std::filesystem::path>;

        [[nodiscard]]
        static auto load(const std::filesystem::path &font_file,
                         float                        pt,
                         std::string_view             style) noexcept -> result<font>;


        [[nodiscard]] auto get_size() noexcept -> float;
        [[nodiscard]] auto get_glyph_metrics(std::uint32_t character) -> metrics;
        [[nodiscard]] auto get_ascent() noexcept -> int;
        [[nodiscard]] auto get_descent() noexcept -> int;
        [[nodiscard]] auto get_height() noexcept -> int;
        [[nodiscard]] auto get_line_skip() noexcept -> int;
        [[nodiscard]] auto get_string_size(const std::string &string) -> size;
        [[nodiscard]] auto is_monospace() noexcept -> bool;

        auto set_size(float pt) noexcept -> result<>;
        auto set_size(float pt, int horizontal_dpi, int vertical_dpi) noexcept -> result<>;
        auto set_style(std::string_view style_string) noexcept -> result<>;

        auto render_glyph(char32_t      glyph,
                          color         fg,
                          color         bg,
                          glyph_quality quality = glyph_quality::blended) noexcept
            -> result<surface>;
    };


    namespace _impl
    {
        inline std::unordered_map<std::filesystem::path, font, heterogeneous_hash> font_library;
    }
}
