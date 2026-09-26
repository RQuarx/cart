#pragma once
#include <expected>
#include <filesystem>
#include <mutex>
#include <thread>

#include <toml++/toml.hpp>

#include "error.hh"


namespace cart
{
    class config
    {
    public:
        [[nodiscard]]
        static auto fetch(const std::filesystem::path &config_file) noexcept
            -> std::expected<std::shared_ptr<config>, error>;


        [[nodiscard]] auto get(std::string_view key) const noexcept -> const toml::node *;


        template <typename T>
        auto get_as(std::string_view key) const noexcept -> const toml::impl::wrap_node<T> *
        { return this->data.get_as<T>(key); }


    private:
        std::mutex            mtx;
        std::filesystem::path config_file;
        toml::table           data;
        std::jthread          config_watcher_thread;


        void reload();
        [[nodiscard]] auto verify() const noexcept -> std::expected<void, error>;
    };
}
