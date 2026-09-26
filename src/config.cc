#include <memory>

#include <spdlog/spdlog.h>

#include "config.hh"
#include "inotify.hh"

using cart::config;


auto config::fetch(const std::filesystem::path &config_file) noexcept
    -> std::expected<std::shared_ptr<config>, error>
{
    std::shared_ptr<config> cfg;

    try
    {
        cfg = std::make_shared<config>();
    }
    catch (const std::exception &e)
    {
        return error { "Failed to allocate memory for config: {}", e.what() }.unexpected();
    }

    if (auto res = toml::parse_file(config_file.string()); res.succeeded())
        cfg->data = std::move(res).table();
    else
        return error { "Failed to parse config file '{}' {}:{}: {}", config_file.c_str(),
                       res.error().source().begin.line, res.error().source().begin.column,
                       res.error().description() }
            .unexpected();

    cfg->config_file = config_file;

    auto fn =
        [cfg](
            std::expected<std::optional<std::reference_wrapper<const ::inotify_event>>, error> res)
    {
        if (res.has_value())
        {
            if (res->has_value())
                spdlog::error("inotify read returned EOF.");
            else
            {
                spdlog::info("Config file modified, reloading config.");
                cfg->reload();
            }
            return;
        }
    };

    if (auto res = create_fs_watcher(config_file, fn); res.has_value())
        cfg->config_watcher_thread = std::move(*res);
    else
        return res.error().unexpected();

    return cfg;
}


void config::reload()
{
    std::scoped_lock lock { this->mtx };

    if (auto res = toml::parse_file(config_file.string()); res.succeeded())
        this->data = std::move(res).table();
    else
        spdlog::error("Failed to parse config file '{}' {}:{}: {}", config_file.c_str(),
                      res.error().source().begin.line, res.error().source().begin.column,
                      res.error().description());
}


auto config::get(std::string_view key) const noexcept -> const toml::node *
{ return this->data.get(key); }
