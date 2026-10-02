#pragma once
#include "terminal/types.hh"


namespace cart::term
{
    struct cursor
    {
        position pos;
        bool     last_column_flag = false;
    };
}
