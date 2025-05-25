#pragma once

#include "atom.hpp"

namespace vkopter::game::citygen
{

template<int32_t S>
class EventWindow
{
public:
    static const int32_t SIZE = S;

    EventWindow() :
        site_memory_{}
    {

    }

    auto operator () (const int32_t x, const int32_t y, const int32_t z = 0) -> Atom&
    {
        return atoms_[x + SIZE][y + SIZE][z + SIZE];
        //return atoms[z+SIZE].at(x+SIZE,y+SIZE);
    }

    auto siteMemory(size_t N, int32_t const x, int32_t const y, int32_t const z = 0) -> int8_t&
    {
        return site_memory_[N][x+SIZE][y+SIZE][z+SIZE];
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
                        if(t == atoms_[x][y][z].type)
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
    Atom atoms_[(SIZE*2)+1][(SIZE*2)+1][(SIZE*2)+1];
    int8_t site_memory_[8][(SIZE*2)+1][(SIZE*2)+1][(SIZE*2)+1];
};

}
