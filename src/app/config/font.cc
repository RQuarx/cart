#include "app/config/font.hh"
#include "app/config/utils.hh"

using cart::app::conf::font;


auto font::parse(const toml::table &font) noexcept -> result<>
try
{
    std::array map {
        std::pair { "normal",      std::ref(m_normal)      },
        std::pair { "bold",        std::ref(m_bold)        },
        std::pair { "italic",      std::ref(m_italic)      },
        std::pair { "bold_italic", std::ref(m_bold_italic) },
    };

    for (const auto &[key, value] : font)
    {
        if (key == "size")
        {
            m_size = utils::as<float>(key, value);
            continue;
        }


        for (const auto &[name, member] : map)
        {
            if (key != name) continue;

            for (const auto &[key_, value_] : utils::as<toml::table>(key, value))
            {
                bool touched = false;

                for (const auto &[name_, pair_member] : {
                         std::pair { "family", family },
                         std::pair { "style",  style  },
                })
                    if (key == name_)
                    {
                        member.get().*pair_member = utils::as<std::string>(key_, value_);
                        touched                   = true;
                        break;
                    }

                if (touched) continue;
                return error { "Unexpected node in font.{}: {}", key.str(), key_.str() }
                    .unexpected();
            }
        }

        return error { "Unexpected node in font: {}", key.str() }.unexpected();
    }

    return {};
}
catch (error &e)
{
    return std::move(e).unexpected();
}
