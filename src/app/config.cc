#include <memory>
#include <ranges>

#include <spdlog/spdlog.h>

#include "app/config.hh"
#include "app/inotify.hh"

using cart::app::config;
using cart::shared::error;


template <>
struct std::formatter<toml::source_position>
{
    constexpr auto parse(auto &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const toml::source_position &pos, FormatContext &ctx) const
        -> FormatContext::iterator
    { return std::format_to(ctx.out(), "{}:{}", pos.line, pos.column); }
};


class config_error : public cart::shared::error
{
public:
    config_error(const toml::parse_error &err)
        : cart::shared::error { "Failed to parse config file {}:[{}]-[{}]: {}", *err.source().path,
                                err.source().begin, err.source().end, err.description() }
    {
    }

    template <typename... Args>
    config_error(const toml::source_region                  &region,
                 cart::shared::_impl::format_string<Args...> fmt,
                 Args &&...args)
        : cart::shared::error {
              "Error [{}:{}]: {}",
              *region.path,
              region.begin,
              std::format(fmt.fmt, std::forward<Args>(args)...),
          }
    {
    }
};


namespace
{
    [[nodiscard]]
    constexpr auto make_dim(std::uint32_t color) noexcept -> std::uint32_t
    { return (color >> 1) & 0x7F7F7F; }


    [[nodiscard]]
    auto read_color(const toml::node &node, std::string_view key) noexcept
        -> cart::result<std::uint32_t>
    {
        const auto v = node.value<std::int64_t>();

        if (!v.has_value())
            return config_error { node.source(), "\"{}\" holds a non-integer value.", key }
                .unexpected();

        if (*v < 0 or *v > 0xFFFFFF)
            return config_error { node.source(), "\"{}\" does not fit a 24-bit rgb value.", key }
                .unexpected();

        return std::uint32_t(*v);
    }


    using triple = std::array<std::uint32_t, 3>; /* bright, normal, dim */


    /** The config value for the color.background/foreground can either be a table, or an int. */
    [[nodiscard]]
    auto read_triple(const toml::node &node, std::string_view key) noexcept -> cart::result<triple>
    {
        if (node.is_integer())
        {
            std::uint32_t normal;
            if (auto res = read_color(node, key); res.has_value())
                normal = *res;
            else
                return res.error().unexpected();

            return triple { normal, normal, make_dim(normal) };
        }

        const auto *table = node.as_table();

        if (table == nullptr)
            return config_error { node.source(), "\"{}\" is not an integer, or a table.", key }
                .unexpected();

        triple t { std::numeric_limits<std::uint32_t>::max() };
        for (const auto &[k, value] : *table)
        {
            std::int8_t index = -1;

            if (k == "bright") index = 0;
            if (k == "normal") index = 1;
            if (k == "dim") index = 2;

            if (index == -1)
                return error { R"(Unrecognized key ("{}") for "{}")", k, key }.unexpected();

            if (auto res = read_color(value, k); res.has_value())
                t[index] = *res;
            else
                return res.error().unexpected();
        }

        return t;
    }


    using octal = std::array<std::uint32_t, 8>;


    [[nodiscard]]
    auto read_octal(const toml::node &node, std::string_view key) noexcept -> cart::result<octal>
    {
        const auto *array = node.as_array();

        if (array == nullptr)
            return config_error { node.source(), "\"{}\" is not an array.", key }.unexpected();
        if (array->size() != 8)
            return config_error { node.source(),
                                  "\"{}\" doesn't have the correct amount of values (8).", key }
                .unexpected();

        octal o { std::numeric_limits<std::uint32_t>::max() };
        for (const auto &[i, value] : std::views::enumerate(*array))
            if (auto res = read_color(value, key); res.has_value())
                o[i] = *res;
            else
                return res.error().unexpected();

        return o;
    }
}


auto config::fetch(const std::filesystem::path &config_file) noexcept
    -> result<std::shared_ptr<config>>
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
        cfg->m_data = std::move(res).table();
    else
    {
        if (!std::filesystem::exists(config_file))
        {
            spdlog::warn("Specified config file ({}) does not exist, using default configuration.",
                         config_file.c_str());

            cfg->m_config_watcher_thread = {};
            cfg->m_config_file           = "";
            cfg->m_data                  = config::get_default();

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


auto config::get_theme() const noexcept -> result<term::theme>
{
    term::theme theme;

    const auto *colors = get("colors");
    if (colors == nullptr) return theme;

    if (!colors->is_table())
        return config_error { colors->source(), "\"colors\" is not of type `table`." }.unexpected();

    for (const auto &[key, val] : {
             std::pair { "foreground", std::ref(theme.foreground) },
             std::pair { "background", std::ref(theme.background) }
    })
        if (const auto *node = colors->as_table()->get(key); node != nullptr)
        {
            if (auto res = read_triple(*node, key); res.has_value())
                val.get() = *res;
            else
                return res.error().unexpected();
        }

    const auto *palette = colors->as_table()->get("palette");

    if (palette == nullptr) return theme;

    for (const auto &[key, node] : *palette->as_table())
        for (const auto &[k, value] : {
                 std::pair { "bright", std::ref(theme.palette[0]) },
                 std::pair { "normal", std::ref(theme.palette[1]) },
                 std::pair { "dim",    std::ref(theme.palette[2]) }
        })
            if (auto res = read_octal(node, k); res.has_value())
                value.get() = *res;
            else
                return res.error().unexpected();

    if (const auto *res = colors->as_table()->get("bold_as_bright"); res != nullptr)
    {
        if (res->is_boolean())
            theme.bold_as_bright = res->as_boolean()->get();
        else
            return config_error { res->source(), "\"bold_as_bright\" is not a boolean." }
                .unexpected();
    }

    return theme;
}


void config::mf_reload()
{
    std::scoped_lock lock { m_mtx };

    if (auto res = toml::parse_file(m_config_file.string()); res.succeeded())
        m_data = std::move(res).table();
    else
        spdlog::error("{}", config_error { res.error() }.what());
}


auto config::get(std::string_view key) const noexcept -> const toml::node *
{ return m_data.get(key); }


auto config::get_default() noexcept -> toml::table
{
    return toml::table {
        { "color",
         toml::table {
              { "bold_as_bright", true },
              {
                  "foreground",
                  toml::table {
                      { "bright", 0xFFFFFF },
                      { "normal", 0xFFFFFF },
                      { "dim", 0xE3C7A1 },
                  },
              },
              {
                  "background",
                  toml::table {
                      { "bright", 0x404040 },
                      { "normal", 0x000000 },
                      { "dim", 0x000000 },
                  },
              },
              {
                  "palette",
                  toml::table {
                      {
                          "bright",
                          toml::array { 0x404040, 0xFF0000, 0x00FF00, 0xFFFF00, 0x0000FF, 0xFF00FF,
                                        0x00FFFF, 0xFFFFFF },
                      },
                      {
                          "normal",
                          toml::array { 0x000000, 0xCD0000, 0x00CD00, 0xCDCD00, 0x0000CD, 0xCD00CD,
                                        0x00CDCD, 0xFAEBD7 },
                      },
                      {
                          "dim",
                          toml::array { 0x000000, 0x872B22, 0x549E4E, 0xAAAB34, 0x0F2353, 0x972596,
                                        0x31A7A6, 0xE3C7A1 },
                      },
                  },
              },
          } }
    };
}
