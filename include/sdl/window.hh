#pragma once
#include "sdl/object.hh"
#include "sdl/pointer.hh"
#include "sdl/renderer.hh"


namespace cart::sdl
{
    class window final : object<>, public uptr<SDL_Window, SDL_DestroyWindow>
    {
    public:
        [[nodiscard]]
        static auto create(const char *title, int w, int h) noexcept
            -> result<std::pair<window, renderer>>;


        [[nodiscard]]
        auto get_renderer(this auto &&self) noexcept -> auto &
        { return self.m_renderer; }


        [[nodiscard]] auto get_id() noexcept -> std::uint64_t;
        [[nodiscard]] auto get_size() -> std::pair<int, int>;
        [[nodiscard]] auto get_size_in_pixels() -> std::pair<int, int>;
        [[nodiscard]] auto get_display_scale() noexcept -> float;
        [[nodiscard]] auto get_pixel_density() noexcept -> float;

        auto start_text_input() noexcept -> result<>;
        auto stop_text_input() noexcept -> result<>;

    private:
        constexpr window(pointer window) noexcept : uptr { window } {}
    };
}
