#pragma once

#include <cstdint>
#include <vector>
#include <random>
#include <ranges>


namespace vkopter::game::citygen
{

enum Type : uint32_t
{
    Empty = 0,
    Grass = 1,

    RoadNS = 2,
    RoadEW = 3,
    RoadDiagBL = 4,
    RoadDiagBR = 5,
    RoadDiagTR = 6,
    RoadDiagTL = 7,

    //intersection types
    Road4Way = 8,
    RoadEWN = 9,
    RoadEWS = 10,
    RoadNSE = 11,
    RoadNSW = 12,

    NUM_TYPES
};

class Atom
{
public:
    Type type = Empty;

    uint32_t data = 0;

    auto static generateRange(uint8_t const first, uint8_t const last) -> std::vector<Type>
    {
        std::vector<Type> r;
        for(auto i = first; i <= last; ++i)
        {
            Type t = static_cast<Type>(i);
            r.push_back(t);
        }
        return r;
    }

    auto operator = (Type const & that) -> Atom&
    {
        type = that;
        return *this;
    }

    auto operator = (std::ranges::range auto atoms) -> Atom&
    {
        static thread_local std::minstd_rand gen(std::random_device{}());

        auto s = 0ul;
        for(auto itr = atoms.begin(); itr!= atoms.end(); ++itr)
        {
            ++s;
        }
        std::uniform_int_distribution<size_t> dis{0,s-1};
        auto r = dis(gen);
        s = 0;
        for(auto itr = atoms.begin(); itr!= atoms.end(); ++itr)
        {
            if(r == s)
            {
                (*this) = (*itr);
            }
            ++s;
        }

        return (*this);

    }

    auto operator << (const Atom& that) -> Atom&
    {
        if(type == Empty)
        {
            (*this) = that;
        }
        return *this;
    }

    auto operator << (const Type& that) -> Atom&
    {
        if(type == Empty)
        {
            type = that;
        }
        return *this;
    }

    auto operator << (std::ranges::range auto atoms) -> Atom&
    {

        if(type != Type::Empty) { return (*this); }

        static thread_local std::minstd_rand gen(std::random_device{}());

        auto s = 0ul;
        for(auto itr = atoms.begin(); itr!= atoms.end(); ++itr)
        {
            ++s;
        }
        std::uniform_int_distribution<size_t> dis{0,s-1};
        auto const r = dis(gen);
        s = 0;
        for(auto itr = atoms.begin(); itr != atoms.end(); ++itr)
        {
            if(r == s)
            {
                (*this) = (*itr);
                return (*this);
            }
            ++s;
        }

        return (*this);

    }

    auto operator == (const Atom& that) const -> bool
    {
        if(that.type == type)
        {
            return true;
        }
        return false;
    }

    auto operator == (const Type& that) const -> bool
    {
        if(that == type)
        {
            return true;
        }
        return false;
    }

    auto operator == (std::ranges::range auto atoms) const -> bool
    {

        for(auto itr = atoms.begin(); itr != atoms.end(); ++itr)
        {
            if((*this) == (*itr))
            {
                return true;
            }
        }

        return false;
    }

    auto operator != (const Atom& that) const -> bool
    {
        return that.type != type;
    }

    auto operator != (const Type& that) const -> bool
    {
        return that != type;
    }

    auto operator != (std::ranges::range auto atoms) const -> bool
    {

        for(auto itr = atoms.begin(); itr != atoms.end(); ++itr)
        {
            if((*this) == (*itr))
            {
                return false;
            }
        }

        return true;
    }

    operator bool () const
    {
        return type != Empty;
    }

    [[nodiscard]] auto isInRange(uint8_t const first, uint8_t const last) const -> bool
    {
        auto const t = static_cast<uint8_t>(this->type);
        if(t >= first && t<= last)
        {
            return true;
        }
        return false;
    }

private:


};

}
