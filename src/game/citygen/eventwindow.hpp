#pragma once

#include "atom.hpp"

#include <iostream>

namespace vkopter::game::citygen
{

template<int32_t S>
class EventWindow
{
public:
    static const int32_t SIZE = S;

    auto operator () (const int32_t x, const int32_t y, const int32_t z = 0) -> Atom&
    {
        //fix adressing for negative x,y,z
        constexpr auto DIM = (S*2)+1;
        int32_t const x2 = x+S;
        int32_t const y2 = y+S;
        int32_t const z2 = z+S;
        auto g = x2 + y2 * DIM + z2 * DIM * DIM;
        return atoms_[x2 + y2 * DIM + z2 * DIM * DIM];
    }

    [[nodiscard]] auto contains(const std::vector<Type>& l) const -> bool
    {
        for(auto& a : atoms_)
        {
            for(auto& lt : l)
            {
                if (a.type == lt)
                {
                    return true;
                }
            }
        }
        return false;
    }



private:
    std::array<Atom, ((SIZE*2)+1)*((SIZE*2)+1)*((SIZE*2)+1)> atoms_;

};

}
