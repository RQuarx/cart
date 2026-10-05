#include "app/config/utils.hh"
#include "app/config/window.hh"

using cart::app::conf::window;


auto window::parse(const toml::table &window) noexcept -> result<>
try
{
    for (const auto &[key, value] : window)
    {
        if (key == "title")
        {
            if (auto new_title = utils::as<std::string>(key, value); !new_title.empty())
                m_title = new_title;
            else
                return error { "Specified title is empty." }.unexpected();
            continue;
        }

        if (key == "padding")
        {
            decltype(m_padding) padding { 0, 0 };

            for (const auto &[key_, value_] : utils::as<toml::table>(key, value))
            {
                if (key_ == "x")
                {
                    padding.x = utils::as<std::uint32_t>(key_, value_);
                    continue;
                }

                if (key_ == "y")
                {
                    padding.y = utils::as<std::uint32_t>(key_, value_);
                    continue;
                }

                return error { "Unexpected node in window.padding: {}", key_.str() }.unexpected();
            }

            continue;
        }

        if (key == "dynamic_padding")
        {
            m_dynamic_padding = utils::as<bool>(key, value);
            continue;
        }

        return error { "Unexpected node in window: {}", key.str() }.unexpected();
    }

    return {};
}
catch (error &e)
{
    return std::move(e).unexpected();
}
