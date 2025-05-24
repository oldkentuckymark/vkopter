#pragma once

#include "atom.hpp"
#include <array>
#include <bit>

namespace vkopter::game::citygen
{

template<int32_t S>
class EventWindow
{
public:
    static const int32_t SIZE = S;

    EventWindow() :
        site_memory{}
    {

    }

    auto operator () (const int32_t x, const int32_t y, const int32_t z = 0) -> Atom&
    {
        return atoms[x + SIZE][y + SIZE][z + SIZE];
        //return atoms[z+SIZE].at(x+SIZE,y+SIZE);
    }

    auto siteMemory(size_t N, int32_t const x, int32_t const y, int32_t const z = 0) -> int8_t&
    {
        return site_memory[N][x+SIZE][y+SIZE][z+SIZE];
    }

    [[nodiscard]] auto contains(const std::vector<Type>& l) const -> bool
    {
        for(auto x = 0; x < SIZE*2+1; ++x)
        {
            for(auto y = 0; y < SIZE*2+1; ++y)
            {
                for(auto z = 0; z < SIZE*2+1; ++z)
                {
                    for(auto const& t : l)
                    {
                        if(t == atoms[x][y][z].type)
                        {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }



private:
    Atom atoms[(SIZE*2)+1][(SIZE*2)+1][(SIZE*2)+1];
    int8_t site_memory[8][(SIZE*2)+1][(SIZE*2)+1][(SIZE*2)+1];
    //std::array< vector2d<(SIZE*2)+1,(SIZE*2)+1,Atom>,(SIZE*2)+1> atoms;
};

}
