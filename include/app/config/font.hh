#pragma once
#include "app/config/base.hh"


namespace cart::app::cfg
{
    class font final : public base
    {
        using string_pair = std::pair<std::string, std::string>;

    public:
        static constexpr auto family = &string_pair::first;
        static constexpr auto style  = &string_pair::first;


        [[nodiscard]]
        static auto get_default() noexcept -> const font &
        {
            static font f {};
            return f;
        }


        auto parse(const toml::table &font) noexcept -> result<> override;


        [[nodiscard]]
        constexpr auto get_normal(std::string string_pair::*type) const noexcept -> std::string_view
        { return m_normal.*type; }

#define FONT_GETTER(name)                                                                            \
        [[nodiscard]]                                                                                \
        constexpr auto get_##name(std::string string_pair::*type) const noexcept -> std::string_view \
        {                                                                                            \
            const std::string &val = m_##name.*type;                                                 \
            return val.empty() ? get_normal(type) : val;                                             \
        }

        FONT_GETTER(bold)
        FONT_GETTER(italic)
        FONT_GETTER(bold_italic)
#undef FONT_GETTER

        [[nodiscard]]
        constexpr auto get_size() const noexcept -> float
        { return m_size; }

    private:
        string_pair m_normal { "JetBrainsMono Nerd Font", "Regular" };
        string_pair m_bold { {}, "Bold" };
        string_pair m_italic { {}, "Italic" };
        string_pair m_bold_italic { {}, "Bold Italic" };

        float m_size = 12.F;
    };
}
