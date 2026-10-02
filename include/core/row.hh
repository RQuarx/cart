#pragma once
#include <vector>

#include "core/cell.hh"


namespace cart::core
{
    struct row
    {
        std::vector<cell> cells;
        bool dirty   = true;
        bool wrapped = false;


        constexpr row(std::size_t columns) : cells { columns } {}
    };
}
