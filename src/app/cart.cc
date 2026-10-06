#include <spdlog/cfg/env.h>
#include <spdlog/spdlog.h>

#include "app/args.hh"
#include "app/cart.hh"
#include "app/window.hh"

using namespace cart;


auto app::cart::run(std::span<char *const> argv) noexcept -> int
{
    spdlog::cfg::load_env_levels();

    struct args args;

    if (auto res = args::parse(argv); res.has_value())
    {
        if (!res->has_value()) return 0;
        args = std::move(**res);
    }
    else
    {
        spdlog::critical("{}", res.error());
        return 1;
    }

    std::shared_ptr<config> config;

    if (auto res = config::parse(args.config_file); res.has_value())
        config = *std::move(res);
    else
    {
        spdlog::critical("{}", res.error());
        return 1;
    }

    if (auto res = window::create(*config); res.has_value())
    {
        cart c { std::move(args), std::move(config), std::move(*res) };

        spdlog::info("Application instance successfuly created,");

        while (true)
            if (auto res = c.on_frame(); res.has_value())
            {
                if (*res != action::continue_process) return std::to_underlying(*res);
            }
            else
            {
                spdlog::critical("{}", res.error());
                return 1;
            }
    }
    else
    {
        spdlog::critical("{}", res.error());
        return 1;
    }

    return 0;
}


app::cart::cart(args &&args, std::shared_ptr<config> &&config, window &&window) noexcept
    : m_args { std::move(args) }, m_window { std::move(window) }, m_renderer { m_window },
      m_config { std::move(config) }
{ m_window.set_on_resize_callback(*this, &cart::mf_on_window_resized); }


auto app::cart::on_frame() noexcept -> result<action>
{
    if (auto res = m_window.on_frame();
        !res or (res.has_value() and *res != action::continue_process))
        return res;

    if (auto res = m_renderer.on_frame(*m_config);
        !res or (res.has_value() and *res != action::continue_process))
        return res;

    return action::continue_process;
}


void app::cart::mf_on_window_resized(int new_width, int new_height) noexcept
{ spdlog::trace("Window resized ({} {})", new_width, new_height); }
