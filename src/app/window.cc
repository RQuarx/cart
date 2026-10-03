#include <algorithm>
#include <string>

#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

#include "app/window.hh"

using cart::app::window;


void window::sdl_window_deleter::operator()(SDL_Window *p) const noexcept
{
    if (p != nullptr) SDL_DestroyWindow(p);
}


void window::sdl_renderer_deleter::operator()(SDL_Renderer *p) const noexcept
{
    if (p != nullptr) SDL_DestroyRenderer(p);
}


window::window(window_ptr w, renderer_ptr r, term::grid g) noexcept
    : m_window { std::move(w) }, m_renderer { std::move(r) }, m_grid { std::move(g) }
{
}


auto window::create(std::string_view title, std::size_t rows, std::size_t columns) noexcept
    -> result<window>
{
    if (rows == 0 or columns == 0)
        return shared::error { "window: dimensions must be non-zero ({}x{})", rows, columns }
            .unexpected();

    if (!SDL_Init(SDL_INIT_VIDEO))
        return shared::error { "SDL_Init failed: {}", SDL_GetError() }.unexpected();

    window_ptr w {
        SDL_CreateWindow(std::string { title }.c_str(), int(columns * k_cell_w),
                         int(rows * k_cell_h), SDL_WINDOW_RESIZABLE),
    };
    if (w == nullptr)
    {
        auto err = shared::error { "SDL_CreateWindow failed: {}", SDL_GetError() };
        SDL_Quit();
        return err.unexpected();
    }

    renderer_ptr r { SDL_CreateRenderer(w.get(), nullptr) };
    if (r == nullptr)
    {
        auto err = shared::error { "SDL_CreateRenderer failed: {}", SDL_GetError() };
        SDL_Quit();
        return err.unexpected();
    }

    auto grid = term::grid::create(rows, columns, k_scrollback);
    if (!grid.has_value())
    {
        SDL_Quit();
        return grid.error().unexpected();
    }

    return window { std::move(w), std::move(r), std::move(*grid) };
}


auto window::run() noexcept -> result<void>
{
    SDL_Event event {};
    bool      running = true;

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT: running = false; break;

            case SDL_EVENT_WINDOW_RESIZED:
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                mf_handle_resize(event.window.data1, event.window.data2);
                break;

            default: break;
            }
        }

        SDL_SetRenderDrawColor(m_renderer.get(), 0, 0, 0, 255);
        SDL_RenderClear(m_renderer.get());
        SDL_RenderPresent(m_renderer.get());
        SDL_Delay(16);
    }

    SDL_Quit();
    return {};
}


auto window::mf_handle_resize(int px_w, int px_h) noexcept -> void
{
    const auto columns = std::size_t(std::max(1, px_w / k_cell_w));
    const auto rows    = std::size_t(std::max(1, px_h / k_cell_h));

    if (rows == m_grid.rows_count() and columns == m_grid.columns()) return;

    if (auto res = m_grid.resize(rows, columns); !res.has_value())
        spdlog::warn("grid::resize({}x{}) failed: {}", rows, columns, res.error().format());
    else
        spdlog::info("resized to {}x{} cells (pushed {}, pulled {})", rows, columns,
                     res->pushed, res->pulled);
}
