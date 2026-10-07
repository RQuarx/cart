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


        struct key
        {
            std::filesystem::path path;
            std::uint32_t         style;
            float                 size;

            friend bool operator==(const key &, const key &) = default;
        };


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
        static auto open(std::string_view family, std::string_view style, float pt) noexcept
            -> result<font>;

        [[nodiscard]] auto get_path() const noexcept -> const std::filesystem::path &;
        [[nodiscard]] auto get_size() noexcept -> float;
        [[nodiscard]] auto get_style() noexcept -> std::uint32_t;
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

        [[nodiscard]] auto as_key() noexcept -> key;

        auto render_glyph(char32_t      glyph,
                          color         fg,
                          color         bg,
                          glyph_quality quality = glyph_quality::blended) noexcept
            -> result<surface>;

    private:
        std::filesystem::path m_font_path;
    };
}


template <>
struct std::hash<cart::sdl::font::key>
{
    [[nodiscard]]
    auto operator()(const cart::sdl::font::key &key) const noexcept -> std::size_t
    { return get_hash_for(key.path, key.style, key.size); }


    [[nodiscard]]
    static auto get_hash_for(const std::filesystem::path &path,
                             std::uint32_t                style,
                             float                        size) noexcept -> std::size_t
    {
        std::size_t hash = 0;

        hash = mf_hash_combine(hash, path);
        hash = mf_hash_combine(hash, style);
        hash = mf_hash_combine(hash, size);

        return hash;
    }

private:
    template <typename T>
    static auto mf_hash_combine(std::size_t seed, const T &value) noexcept -> std::size_t
    {
        seed ^= std::hash<T> {}(value) + 0x9E3779B97f4A7C15ULL + (seed << 6) + (seed >> 2);
        return seed;
    }
};
