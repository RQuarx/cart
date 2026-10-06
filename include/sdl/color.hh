#pragma once
#include <concepts>
#include <cstdint>

#include <SDL3/SDL_pixels.h>


namespace cart::sdl
{
    struct color
    {
        float r;
        float g;
        float b;
        float a;


        [[nodiscard]]
        static constexpr auto from24(std::uint32_t rgb, float a = 1.F) noexcept -> color
        {
            float r = static_cast<float>((rgb >> 16) & 0xFF) / 255.0F;
            float g = static_cast<float>((rgb >> 8) & 0xFF) / 255.0F;
            float b = static_cast<float>(rgb & 0xFF) / 255.0F;

            return { r, g, b, a };
        }


        [[nodiscard]]
        constexpr auto to_fcolor() const noexcept -> SDL_FColor
        { return *reinterpret_cast<const SDL_FColor *>(this); }


        template <std::invocable<float, float, float, float> F>
        [[nodiscard]]
        auto invoke(F &&fn)
        { return fn(r, g, b, a); }
    };
}
