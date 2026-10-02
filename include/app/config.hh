#pragma once
#include <filesystem>
#include <mutex>
#include <thread>

#include <toml++/toml.hpp>

#include "terminal/theme.hh"
#include "shared/result.hh"


namespace cart::app
{
    class config
    {
    public:
        [[nodiscard]]
        static auto fetch(const std::filesystem::path &config_file) noexcept
            -> result<std::shared_ptr<config>>;


        [[nodiscard]] auto get(std::string_view key) const noexcept -> const toml::node *;


        template <typename T>
        auto get_as(std::string_view key) const noexcept -> const toml::impl::wrap_node<T> *
        { return m_data.get_as<T>(key); }


        [[nodiscard]] auto get_theme() const noexcept -> result<term::theme>;


    private:
        std::mutex            m_mtx;
        std::filesystem::path m_config_file;
        toml::table           m_data;
        std::jthread          m_config_watcher_thread;


        [[nodiscard]]
        static auto get_default() noexcept -> toml::table;

        void               mf_reload();
        [[nodiscard]] auto mf_verify() const noexcept -> result<void>;
    };
}
