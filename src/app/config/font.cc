#include "app/config/font.hh"
#include "app/config/utils.hh"

using cart::app::cfg::font;


auto font::parse(const toml::table &font) noexcept -> result<>
try
{
    std::array faces {
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

        auto *const it = std::ranges::find(faces, std::string_view { key },
                                           &decltype(faces)::value_type::first);
        if (it == faces.end())
            return error { "Unexpected node in font: {}", key.str() }.unexpected();

        for (const auto &[key_, value_] : utils::as<toml::table>(key, value))
        {
            if (key_ == "family")
                it->second.get().*family = utils::as<std::string>(key_, value_);
            else if (key_ == "style")
                it->second.get().*style = utils::as<std::string>(key_, value_);
            else
                return error { "Unexpected node in font.{}: {}", key.str(), key_.str() }
                    .unexpected();
        }
    }

    return {};
}
catch (error &e)
{
    return std::move(e).unexpected();
}
