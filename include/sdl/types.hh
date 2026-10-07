#pragma once
#include <algorithm>
#include <cstdint>

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>


namespace cart::sdl
{
    struct color final
    {
        float r, g, b, a;


        [[nodiscard]]
        static constexpr auto from24(std::uint32_t rgb, float a = 1.F) noexcept -> color
        {
            float r = static_cast<float>((rgb >> 16) & 0xFF) / 255.0F;
            float g = static_cast<float>((rgb >> 8) & 0xFF) / 255.0F;
            float b = static_cast<float>(rgb & 0xFF) / 255.0F;

            return { r, g, b, a };
        }


        [[nodiscard]]
        constexpr auto to_fcolor() const noexcept -> const SDL_FColor &
        { return *reinterpret_cast<const SDL_FColor *>(this); }


        [[nodiscard]]
        constexpr auto to_color() const noexcept -> SDL_Color
        {
            auto float_to_u8 = [](float val)
            {
                float scaled = (val * 255.0F) + 0.5F;
                return std::uint8_t(std::clamp(scaled, 0.0F, 255.0F));
            };

            return { float_to_u8(r), float_to_u8(g), float_to_u8(b), float_to_u8(a) };
        }
    };


    struct point final
    {
        float x, y;


        [[nodiscard]]
        constexpr auto to_fpoint() const noexcept -> const SDL_FPoint &
        { return *reinterpret_cast<const SDL_FPoint *>(this); }
    };


    struct size final
    { float w, h; };


    struct rect final
    {
        sdl::point pos;
        sdl::size  size;


        constexpr rect(point p, struct size s) noexcept : pos { p }, size { s } {}
        constexpr rect(float x, float y, float w, float h) noexcept : pos { x, y }, size { w, h } {}

        [[nodiscard]]
        constexpr auto to_frect() const noexcept -> const SDL_FRect &
        { return *reinterpret_cast<const SDL_FRect *>(this); }
    };
}
