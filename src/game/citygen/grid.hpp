#pragma once

#include "atom.hpp"
#include "eventwindow.hpp"
#include <array>

namespace vkopter::game::citygen
{

template<int32_t W, int32_t H, int32_t D>
class Grid
{
public:

    auto operator()(const int32_t x, const int32_t y, const int32_t z = 0) -> Atom&
    {
        return atoms[x][y][z];
        //return atoms[z].at(x,y);
    }

    template<int32_t S>
    auto copyEW(int32_t const x, int32_t const y, int32_t const z = 0) -> EventWindow<S>
    {
        EventWindow<S> win;


        for(int32_t ewx = -EventWindow<S>::SIZE; ewx <= EventWindow<S>::SIZE; ++ewx)
        {
            for(int32_t ewy = -EventWindow<S>::SIZE; ewy <= EventWindow<S>::SIZE; ++ewy)
            {
                for(int32_t ewz = -EventWindow<S>::SIZE; ewz <= EventWindow<S>::SIZE; ++ewz)
                {
                    if(inBounds(ewx+x, ewy+y,ewz+z))
                    {
                        win(ewx,ewy,ewz) = (*this)(ewx+x, ewy+y,ewz+z);
                        win.siteMemory(0,ewx,ewy,ewz) = siteMemory(0,ewx+x, ewy+y,ewz+z);
                        win.siteMemory(1,ewx,ewy,ewz) = siteMemory(1,ewx+x, ewy+y,ewz+z);
                        win.siteMemory(2,ewx,ewy,ewz) = siteMemory(2,ewx+x, ewy+y,ewz+z);
                        win.siteMemory(3,ewx,ewy,ewz) = siteMemory(3,ewx+x, ewy+y,ewz+z);
                        win.siteMemory(4,ewx,ewy,ewz) = siteMemory(4,ewx+x, ewy+y,ewz+z);
                        win.siteMemory(5,ewx,ewy,ewz) = siteMemory(5,ewx+x, ewy+y,ewz+z);
                        win.siteMemory(6,ewx,ewy,ewz) = siteMemory(6,ewx+x, ewy+y,ewz+z);
                        win.siteMemory(7,ewx,ewy,ewz) = siteMemory(7,ewx+x, ewy+y,ewz+z);
                    }
                    else
                    {
                        win(ewx,ewy,ewz) = Empty;
                        win.siteMemory(0,ewx,ewy,ewz) = 0;
                        win.siteMemory(1,ewx,ewy,ewz) = 0;
                        win.siteMemory(2,ewx,ewy,ewz) = 0;
                        win.siteMemory(3,ewx,ewy,ewz) = 0;
                        win.siteMemory(4,ewx,ewy,ewz) = 0;
                        win.siteMemory(5,ewx,ewy,ewz) = 0;
                        win.siteMemory(6,ewx,ewy,ewz) = 0;
                        win.siteMemory(7,ewx,ewy,ewz) = 0;
                    }
                }
            }
        }

        return win;
    }

    template<int32_t S>
    auto pasteEW(EventWindow<S>& ew, int32_t const x, int32_t const y, int32_t const z = 0) -> void
    {
        for(int32_t ewx = -EventWindow<S>::SIZE; ewx < EventWindow<S>::SIZE; ++ewx)
        {
            for(int32_t ewy = -EventWindow<S>::SIZE; ewy < EventWindow<S>::SIZE; ++ewy)
            {
                for(int32_t ewz = -EventWindow<S>::SIZE; ewz < EventWindow<S>::SIZE; ++ewz)
                {
                    if(inBounds(x + ewx,y + ewy,z + ewz))
                    {
                        (*this)( x + ewx, y + ewy, z + ewz) =  ew(ewx,ewy,ewz);
                        siteMemory(0, x + ewx, y + ewy, z + ewz) =  ew.siteMemory(0,ewx,ewy,ewz);
                        siteMemory(1, x + ewx, y + ewy, z + ewz) =  ew.siteMemory(1,ewx,ewy,ewz);
                        siteMemory(2, x + ewx, y + ewy, z + ewz) =  ew.siteMemory(2,ewx,ewy,ewz);
                        siteMemory(3, x + ewx, y + ewy, z + ewz) =  ew.siteMemory(3,ewx,ewy,ewz);
                        siteMemory(4, x + ewx, y + ewy, z + ewz) =  ew.siteMemory(4,ewx,ewy,ewz);
                        siteMemory(5, x + ewx, y + ewy, z + ewz) =  ew.siteMemory(5,ewx,ewy,ewz);
                        siteMemory(6, x + ewx, y + ewy, z + ewz) =  ew.siteMemory(6,ewx,ewy,ewz);
                        siteMemory(7, x + ewx, y + ewy, z + ewz) =  ew.siteMemory(7,ewx,ewy,ewz);
                    }
                }
            }
        }
    }

    auto inBounds(const int32_t x, const int32_t y, const int32_t z = 0) const -> bool
    {
        return x <= WIDTH - 1 &&
                x >= 0 &&
                y <= HEIGHT - 1 &&
                y >= 0 &&
                z <= DEPTH - 1 &&
                z >= 0;
    }

    auto clear(const Type& t = Empty) -> void
    {
        for(auto x = 0; x < WIDTH; ++x)
        {
            for(auto y = 0; y < HEIGHT; ++y)
            {
                for(auto z = 0; z < DEPTH; ++z)
                {
                    atoms[x][y][z] = t;
                    site_memory[0][x][y][z] = 0;
                    site_memory[1][x][y][z] = 0;
                    site_memory[2][x][y][z] = 0;
                    site_memory[3][x][y][z] = 0;
                    site_memory[4][x][y][z] = 0;
                    site_memory[5][x][y][z] = 0;
                    site_memory[6][x][y][z] = 0;
                    site_memory[7][x][y][z] = 0;
                }
            }
        }
    }


    auto siteMemory(size_t const N, int32_t const x, int32_t const y, int32_t const z = 0) -> int8_t&
    {
        return site_memory[N][x][y][z];
    }


    int32_t const static WIDTH = W;
    int32_t const static HEIGHT = H;
    int32_t const static DEPTH = D;

private:
    Atom atoms[WIDTH][HEIGHT][DEPTH];
    int8_t site_memory[8][WIDTH][HEIGHT][DEPTH];

};

}

