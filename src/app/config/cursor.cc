#include "app/config/cursor.hh"
#include "app/config/utils.hh"

using cart::app::conf::cursor;


auto cursor::parse(const toml::table &cursor) noexcept -> result<>
try
{
    using std::chrono::milliseconds;
    using std::chrono::seconds;


    for (const auto &[key, value] : cursor)
    {
        if (key == "shape")
        {
            if (auto res = string_to_shape(utils::as<std::string>(key, value)); res.has_value())
                m_shape = *res;
            else
                throw error {
                    R"(Cursor shape is invalid, available shapes are "beam", "underline", "block".)"
                };

            continue;
        }

        if (key == "blinking")
        {
            if (auto res = string_to_blinking_mode(utils::as<std::string>(key, value));
                res.has_value())
                m_blinking = *res;
            else
                throw error {
                    R"(Cursor blinking mode is invalid, available modes are "never", "off", "on", "always".)"
                };
            continue;
        }

        if (key == "blink_interval")
        {
            m_blink_interval = milliseconds(utils::as<int>(key, value));
            continue;
        }

        if (key == "blink_timeout")
        {
            m_blink_timeout = seconds(utils::as<int>(key, value));
            continue;
        }

        if (key == "unfocused_hollow")
        {
            m_unfocused_hollow = utils::as<bool>(key, value);
            continue;
        }

        if (key == "thickness")
        {
            m_thickness = utils::as<float>(key, value);
            continue;
        }


        return error { "Unexpected node in cursor: {}", key.str() }.unexpected();
    }

    return {};
}
catch (error &e)
{
    return std::move(e).unexpected();
}
