#pragma once
#include "app/config/base.hh"


namespace cart::app::cfg
{
    using namespace std::chrono_literals;

    class cursor final : public base
    {
    public:
        enum class shape : std::uint8_t
        {
            beam,
            block,
            underline,
        };

        enum class blinking_mode : std::uint8_t
        {
            never,
            off,
            on,
            always,
        };


        [[nodiscard]]
        static constexpr auto get_default() noexcept -> const cursor &
        {
            static constexpr cursor c {};
            return c;
        }


        auto parse(const toml::table &cursor) noexcept -> result<> override;


        [[nodiscard]]
        constexpr auto get_blink_interval() const noexcept -> std::chrono::milliseconds
        { return m_blink_interval; }

        [[nodiscard]]
        constexpr auto get_blink_timeout() const noexcept -> std::chrono::seconds
        { return m_blink_timeout; }

        [[nodiscard]]
        constexpr auto get_thickness() const noexcept -> float
        { return m_thickness; }

        [[nodiscard]]
        constexpr auto get_unfocused_hollow() const noexcept -> bool
        { return m_unfocused_hollow; }

        [[nodiscard]]
        constexpr auto get_shape() const noexcept -> shape
        { return m_shape; }

        [[nodiscard]]
        constexpr auto get_blinking() const noexcept -> blinking_mode
        { return m_blinking; }

    private:
        std::chrono::milliseconds m_blink_interval   = 740ms;
        std::chrono::seconds      m_blink_timeout    = 5s;
        float                     m_thickness        = 0.15F;
        bool                      m_unfocused_hollow = true;
        shape                     m_shape            = shape::beam;
        blinking_mode             m_blinking         = blinking_mode::on;


        [[nodiscard]]
        static constexpr auto string_to_shape(std::string_view string) noexcept
            -> std::optional<shape>
        {
            if (string == "beam") return shape::beam;
            if (string == "block") return shape::block;
            if (string == "underline") return shape::underline;
            return std::nullopt;
        }


        [[nodiscard]]
        static constexpr auto string_to_blinking_mode(std::string_view string) noexcept
            -> std::optional<blinking_mode>
        {
            if (string == "never") return blinking_mode::never;
            if (string == "off") return blinking_mode::off;
            if (string == "on") return blinking_mode::on;
            if (string == "always") return blinking_mode::always;
            return std::nullopt;
        }
    };
}
