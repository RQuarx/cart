#pragma once
#include <filesystem>
#include <mutex>
#include <thread>

#include <toml++/toml.hpp>

#include "app/config/colors.hh"
#include "app/config/cursor.hh"
#include "app/config/font.hh"
#include "app/config/scrolling.hh"
#include "app/config/terminal.hh"
#include "app/config/window.hh"
#include "config/base.hh"
#include "shared/callback.hh"
#include "shared/hash.hh"
#include "shared/result.hh"


namespace cart::app
{
    class config
    {
    public:
        using config_changed_callback = callback<void()>;


        [[nodiscard]]
        static auto parse(const std::filesystem::path &config_file) noexcept
            -> result<std::shared_ptr<config>>;

        config();

        template <typename Self>
        void set_config_changed_callback(Self &self, void (Self::*fn)())
        {
            std::scoped_lock lock { m_mtx };
            m_config_changed_callback.set(self, fn);
        }


        [[nodiscard]] auto get_colors() const noexcept -> const cfg::colors &;
        [[nodiscard]] auto get_cursor() const noexcept -> const cfg::cursor &;
        [[nodiscard]] auto get_font() const noexcept -> const cfg::font &;
        [[nodiscard]] auto get_scrolling() const noexcept -> const cfg::scrolling &;
        [[nodiscard]] auto get_terminal() const noexcept -> const cfg::terminal &;
        [[nodiscard]] auto get_window() const noexcept -> const cfg::window &;


    private:
        std::mutex              m_mtx;
        config_changed_callback m_config_changed_callback;
        std::jthread            m_config_watcher_thread;

        std::filesystem::path m_config_file;

        std::unordered_map<std::string, std::unique_ptr<cfg::base>, heterogeneous_hash> m_configs;


        auto mf_reload() noexcept -> result<>;
    };
}
