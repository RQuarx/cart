#include "app/config/scrolling.hh"
#include "app/config/utils.hh"

using cart::app::conf::scrolling;


auto scrolling::parse(const toml::table &scrolling) noexcept -> result<>
try
{
    for (const auto &[key, value] : scrolling)
    {
        if (key == "scrollback")
        {
            m_scrollback = utils::as<std::size_t>(key, value);
            continue;
        }

        if (key == "multiplier")
        {
            m_multiplier = utils::as<std::uint8_t>(key, value);
            continue;
        }

        return error { "Unexpected node in scrolling: {}", key.str() }.unexpected();
    }

    return {};
}
catch (error &e)
{
    return std::move(e).unexpected();
}