#pragma once
#include <chrono>

#include "terminal/types.hh"


namespace cart::term
{
    using namespace std::chrono_literals;


    struct cursor
    {
        enum class shape : std::uint8_t
        {
            beam,
            underline,
            block
        };

        enum class blink_mode : std::uint8_t
        {
            never,
            off,
            on,
            always
        };

        struct attribute
        {
            std::chrono::milliseconds blink_interval;
            std::chrono::seconds      blink_timeout;
            float                     thickness;
            shape                     cursor_shape;
            blink_mode                cursor_blink_mode;
            bool                      unfocused_hollow;
            bool                      last_column_flag = false;
        } attribute;

        position pos;
    };
}
