#include <memory>
#include <ranges>

#include <spdlog/spdlog.h>

#include "app/config.hh"
#include "app/config/utils.hh"
#include "app/inotify.hh"

using cart::error;
using cart::app::config;


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

    template <typename... Args>
    config_error(const toml::source_region          &region,
                 cart::_impl::format_string<Args...> fmt,
                 Args and...args)
        : cart::error {
              "Error [{}:{}]: {}",
              *region.path,
              region.begin,
              std::format(fmt.fmt, std::forward<Args>(args)...),
          }
    {
    }
};


auto config::parse(const std::filesystem::path &config_file) noexcept
    -> result<std::shared_ptr<config>>
try
{
    auto cfg = std::make_shared<config>();

    if (toml::parse_result res = toml::parse_file(config_file.string()); res.succeeded())
    {
        for (const auto &[key, conf] : cfg->m_configs)
            if (auto table = conf::utils::get<toml::table>(res.table(), key); table.has_value())
            {
                spdlog::info("Parsing config for {}.", key);
                if (auto res = conf->parse(*table); !res) return res.error().unexpected();
            }
    }
    else
    {
        if (!std::filesystem::exists(config_file))
        {
            spdlog::warn("Specified config file ({}) does not exist, using default configuration.",
                         config_file.c_str());

            cfg->m_config_watcher_thread = {};
            cfg->m_config_file           = "";

            return cfg;
        }

        return config_error { res.error() }.unexpected();
    }

    cfg->m_config_file = config_file;

    auto fn = [cfg](result<std::reference_wrapper<const ::inotify_event>> res)
    {
        if (res.has_value())
        {
            std::string_view name { res->get().name, res->get().len };

            spdlog::info("Config file {} has been modified, reloading config.", name);
            cfg->mf_reload();
            return;
        }

        spdlog::error("{}", res.error());
    };

    if (auto res = create_fs_watcher(config_file, fn); res.has_value())
        cfg->m_config_watcher_thread = std::move(*res);
    else
        return res.error().unexpected();

    return cfg;
}
catch (const std::bad_alloc &e)
{
    return error { "Failed to allocate memory for config: {}", e.what() }.unexpected();
}


config::config()
{
    m_configs.reserve(5);
    m_configs.emplace("colors", std::make_unique<conf::colors>());
    m_configs.emplace("cursor", std::make_unique<conf::cursor>());
    m_configs.emplace("font", std::make_unique<conf::font>());
    m_configs.emplace("scrolling", std::make_unique<conf::scrolling>());
    m_configs.emplace("terminal", std::make_unique<conf::terminal>());
    m_configs.emplace("window", std::make_unique<conf::window>());
}


void config::mf_reload()
{
    std::scoped_lock lock { m_mtx };

    if (toml::parse_result res = toml::parse_file(m_config_file.string()); res.succeeded())
    {
        for (const auto &[key, conf] : m_configs)
            if (auto table = conf::utils::get<toml::table>(res.table(), key); table.has_value())
            {
                spdlog::info("Reparsing config for {}.", key);
                if (auto res = conf->parse(*table); !res) spdlog::error("{}", res.error());
            }
    }
    else
        spdlog::error("{}", config_error { res.error() }.what());

    m_config_changed_callback();
}


auto config::get_colors() const noexcept -> const conf::colors &
{ return dynamic_cast<const conf::colors &>(*m_configs.find("colors")->second); }


auto config::get_cursor() const noexcept -> const conf::cursor &
{ return dynamic_cast<const conf::cursor &>(*m_configs.find("cursor")->second); }


auto config::get_font() const noexcept -> const conf::font &
{ return dynamic_cast<const conf::font &>(*m_configs.find("font")->second); }


auto config::get_scrolling() const noexcept -> const conf::scrolling &
{ return dynamic_cast<const conf::scrolling &>(*m_configs.find("scrolling")->second); }


auto config::get_terminal() const noexcept -> const conf::terminal &
{ return dynamic_cast<const conf::terminal &>(*m_configs.find("terminal")->second); }


auto config::get_window() const noexcept -> const conf::window &
{ return dynamic_cast<const conf::window &>(*m_configs.find("window")->second); }

