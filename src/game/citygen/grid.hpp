#pragma once

#include "atom.hpp"
#include "eventwindow.hpp"
#include "../terrain.hpp"

namespace vkopter::game::citygen
{

template<int32_t W, int32_t H, int32_t D>
class Grid
{
public:
    static constexpr decltype(W) WIDTH = W;
    static constexpr decltype(H) HEIGHT = H;
    static constexpr decltype(D) DEPTH = D;

    enum SiteLayer : uint8_t
    {
        TerrainType = 0,
        Altitude,
        Crime,
        Economy,
        Pollution

    };

    auto operator()(const int32_t x, const int32_t y, const int32_t z = 0) -> Atom&
    {
        return atoms_[x + y * W + z * W * H];
    }

    template<int32_t S>
    auto copyEW(int32_t const x, int32_t const y, int32_t const z = 0) -> EventWindow<S>
    {
        EventWindow<S> win, win2, win3;

        for(int32_t ewx = -EventWindow<S>::SIZE; ewx <= EventWindow<S>::SIZE; ++ewx)
        {
            for(int32_t ewy = -EventWindow<S>::SIZE; ewy <= EventWindow<S>::SIZE; ++ewy)
            {
                for(int32_t ewz = -EventWindow<S>::SIZE; ewz <= EventWindow<S>::SIZE; ++ewz)
                {
                    if(isInBounds(ewx+x, ewy+y,ewz+z))
                    {
                        win(ewx,ewy,ewz) = (*this)(ewx+x, ewy+y,ewz+z);
                        //win.siteMemory(0,ewx,ewy,ewz) = siteMemory(0,ewx+x, ewy+y,ewz+z);
                        //win.siteMemory(1,ewx,ewy,ewz) = siteMemory(1,ewx+x, ewy+y,ewz+z);

                    }
                    else
                    {
                        win(ewx,ewy,ewz) = Empty;
                        //win.siteMemory(0,ewx,ewy,ewz) = 0;
                        //win.siteMemory(1,ewx,ewy,ewz) = 0;

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
                    if(isInBounds(x + ewx,y + ewy,z + ewz))
                    {
                        (*this)( x + ewx, y + ewy, z + ewz) =  ew(ewx,ewy,ewz);
                        // siteMemory(0, x + ewx, y + ewy, z + ewz) =  ew.siteMemory(0,ewx,ewy,ewz);
                        // siteMemory(1, x + ewx, y + ewy, z + ewz) =  ew.siteMemory(1,ewx,ewy,ewz);
                        // siteMemory(2, x + ewx, y + ewy, z + ewz) =  ew.siteMemory(2,ewx,ewy,ewz);
                        // siteMemory(3, x + ewx, y + ewy, z + ewz) =  ew.siteMemory(3,ewx,ewy,ewz);
                        // siteMemory(4, x + ewx, y + ewy, z + ewz) =  ew.siteMemory(4,ewx,ewy,ewz);
                        // siteMemory(5, x + ewx, y + ewy, z + ewz) =  ew.siteMemory(5,ewx,ewy,ewz);
                        // siteMemory(6, x + ewx, y + ewy, z + ewz) =  ew.siteMemory(6,ewx,ewy,ewz);
                        // siteMemory(7, x + ewx, y + ewy, z + ewz) =  ew.siteMemory(7,ewx,ewy,ewz);
                    }
                }
            }
        }
    }

    auto isInBounds(const int32_t x, const int32_t y, const int32_t z = 0) const -> bool
    {
        return x <= W - 1 &&
                x >= 0 &&
                y <= H - 1 &&
                y >= 0 &&
                z <= D - 1 &&
                z >= 0;
    }

    auto clear(const Type& t = Empty) -> void
    {
        std::fill(atoms_.begin(),atoms_.end(),Atom{t,0});
    }


    // auto siteMemory(size_t const N, int32_t const x, int32_t const y, int32_t const z = 0) -> int8_t&
    // {
    //     return site_memory_[N][x][y][z];
    // }

    auto loadTerrain(Terrain & terrain) -> void
    {
        for(auto x = 0ul; x < W; ++x)
        {
            for(auto y = 0ul; y < H; ++y)
            {
                //site_memory_[SiteLayer::TerrainType][x][y][0] = terrain.termap().at(x,y);
                //site_memory_[SiteLayer::Altitude][x][y][0] = terrain.altmap().at(x,y);
            }
        }

    }

    auto atoms() -> std::array<Atom,W*H*D>&
    {
        return atoms_;
    }

private:
    std::array<Atom, W*H*D> atoms_;

    // int ThreeToOne (int x, int y, int z, int xSize, int ySize, int zSize) {
    //     return x + y * xSize + z * xSize * ySize;
    // }
    // void OneToThree (int index, int xSize,int ySize, int zSize)
    // {
    //     int resultx, resulty, resultz;
    //     resultx = index % xSize;
    //     resulty = (index / xSize) % ySize;
    //     resultz = index / (xSize * ySize);
    //     //return result;
    // }

};

}

