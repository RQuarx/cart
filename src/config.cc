#include <memory>

#include <spdlog/spdlog.h>

#include "config.hh"
#include "inotify.hh"

using cart::config;


template <>
struct std::formatter<toml::source_position>
{
    constexpr auto parse(auto &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const toml::source_position &pos, FormatContext &ctx) const
        -> FormatContext::iterator
    { return std::format_to(ctx.out(), "{}:{}", pos.line, pos.column); }
};


class config_error : public cart::error
{
public:
    config_error(const toml::parse_error &err)
        : cart::error { "Failed to parse config file {}:[{}]-[{}]: {}", *err.source().path,
                        err.source().begin, err.source().end, err.description() }
    {
    }
};


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
    {
        if (!std::filesystem::exists(config_file))
        {
            spdlog::warn("Specified config file ({}) does not exist, using default configuration.",
                         config_file.c_str());

            cfg->config_watcher_thread = {};
            cfg->config_file           = "";
            cfg->data                  = config::get_default();

            return cfg;
        }

        return config_error { res.error() }.unexpected();
    }

    cfg->config_file = config_file;

    auto fn = [cfg](std::expected<std::reference_wrapper<const ::inotify_event>, error> res)
    {
        if (res.has_value())
        {
            spdlog::info("Config file modified, reloading config.");
            cfg->reload();
            return;
        }

        spdlog::error("{}", res.error());
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
        spdlog::error("{}", config_error { res.error() }.what());
}


auto config::get(std::string_view key) const noexcept -> const toml::node *
{ return this->data.get(key); }


auto config::get_default() noexcept -> toml::table
{
    toml::table config;


    return config;
}
