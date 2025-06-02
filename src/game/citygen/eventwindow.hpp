#pragma once

#include "atom.hpp"

namespace vkopter::game::citygen
{

template<int32_t S>
class EventWindow
{
public:
    static const int32_t SIZE = S;

    auto operator () (const int32_t x, const int32_t y, const int32_t z = 0) -> Atom&
    {

        return atoms_[x + y * S + z * S * S];

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
    std::array<Atom, ((SIZE*2)+1)*3> atoms_;

};

}
