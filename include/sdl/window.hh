#pragma once
#include "sdl/renderer.hh"
#include "sdl/runtime_guard.hh"
#include "shared/traits.hh"


namespace cart::sdl
{
    class window final : runtime_guard<>,
                         public trait::unique_handle_of<SDL_Window, SDL_DestroyWindow>
    {
    public:
        using unique_handle_of::unique_handle_of;

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
    };
}
