#pragma once
#include <memory>
#include <string_view>

#include "shared/result.hh"
#include "terminal/grid.hh"


struct SDL_Window;
struct SDL_Renderer;


namespace cart::app
{
    class window
    {
    public:
        [[nodiscard]]
        static auto create(std::string_view title,
                           std::size_t      rows    = 24,
                           std::size_t      columns = 80) noexcept -> result<window>;

        auto run() noexcept -> result<void>;

        window(window &&) noexcept                     = default;
        auto operator=(window &&) noexcept -> window & = default;

        window(const window &)                     = delete;
        auto operator=(const window &) -> window & = delete;

    private:
        struct sdl_window_deleter
        { void operator()(struct SDL_Window *p) const noexcept; };

        struct sdl_renderer_deleter
        { void operator()(struct SDL_Renderer *p) const noexcept; };

        using window_ptr   = std::unique_ptr<struct SDL_Window, sdl_window_deleter>;
        using renderer_ptr = std::unique_ptr<struct SDL_Renderer, sdl_renderer_deleter>;


        static constexpr int k_cell_w = 9;
        static constexpr int k_cell_h = 18;

        static constexpr std::size_t k_scrollback = 1000;


        window_ptr   m_window;
        renderer_ptr m_renderer;
        term::grid   m_grid;


        window(window_ptr w, renderer_ptr r, term::grid g) noexcept;

        auto mf_handle_resize(int px_w, int px_h) noexcept -> void;
    };
}
