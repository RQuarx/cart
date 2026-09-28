#pragma once
#include <expected>
#include <filesystem>
#include <mutex>
#include <thread>

#include <toml++/toml.hpp>

#include "core/theme.hh"
#include "shared/error.hh"


namespace cart::app
{
    class config
    {
    public:
        [[nodiscard]]
        static auto fetch(const std::filesystem::path &config_file) noexcept
            -> std::expected<std::shared_ptr<config>, shared::error>;


        [[nodiscard]] auto get(std::string_view key) const noexcept -> const toml::node *;


        template <typename T>
        auto get_as(std::string_view key) const noexcept -> const toml::impl::wrap_node<T> *
        { return this->data.get_as<T>(key); }


        [[nodiscard]] auto get_theme() const noexcept -> std::expected<core::theme, shared::error>;


    private:
        std::mutex            mtx;
        std::filesystem::path config_file;
        toml::table           data;
        std::jthread          config_watcher_thread;


        [[nodiscard]]
        static auto get_default() noexcept -> toml::table;

        void               reload();
        [[nodiscard]] auto verify() const noexcept -> std::expected<void, shared::error>;
    };
}
