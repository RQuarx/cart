#pragma once
#include "app/window.hh"
#include "sdl/font.hh"
#include "sdl/renderer.hh"


namespace cart::app::render
{
    class renderer
    {
        enum class font_style : std::uint8_t
        {
            normal,
            bold,
            italic,
            bold_italic
        };

    public:
        [[nodiscard]]
        static auto create(window &window, const config &config) noexcept -> result<renderer>;


        [[nodiscard]]
        auto on_frame(const config &config) noexcept -> result<action>;

        [[nodiscard]]
        auto on_config_changed(const config &config) noexcept -> result<>;

    private:
        sdl::renderer m_renderer;

        std::array<sdl::font, 4> m_fonts;


        auto mf_load_fonts(const cfg::font &font_config) noexcept -> result<>;
    };
}
