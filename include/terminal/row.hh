#pragma once
#include <vector>

#include "terminal/cell.hh"


namespace cart::term
{
    struct row
    {
        std::vector<cell> cells;
        bool dirty   = true;
        bool wrapped = false;


        constexpr row(std::size_t columns) : cells { columns } {}
    };
}
