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
        : cart::shared::error { "Error [{}:{}]: {}", *region.path, region.begin,
                                std::format(fmt.fmt, std::forward<Args>(args)...) }
    {
    }
};


namespace
{
    [[nodiscard]]
    constexpr auto make_dim(std::uint32_t color) noexcept -> std::uint32_t
    { return (color << 1) & 0x7F7F7F; }


    [[nodiscard]]
    auto read_color(const toml::node &node, std::string_view key) noexcept
        -> std::expected<std::uint32_t, cart::shared::error>
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
    auto read_triple(const toml::node &node, std::string_view key) noexcept
        -> std::expected<triple, cart::shared::error>
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
    auto read_octal(const toml::node &node, std::string_view key) noexcept
        -> std::expected<octal, cart::shared::error>
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
            std::string_view name { res->get().name, res->get().len };

            spdlog::info("Config file {} has been modified, reloading config.", name);
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


auto config::get_theme() const noexcept -> std::expected<core::theme, error>
{
    core::theme theme;

    const auto *colors = this->get("colors");
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
