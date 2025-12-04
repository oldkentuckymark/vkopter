#pragma once

#include <bit>
#include <string>
#include "util/array2d.hpp"
#include <SDL2/SDL.h>
#include <glm/vec2.hpp>

namespace vkopter::game
{


class Terrain
{
public:
    explicit Terrain()
    {
        clear();

    }
    explicit Terrain(const std::string &path)
    {
        load(path);
    }


    void load(std::string const & path)
    {

        SDL_Surface* surf = SDL_LoadBMP(path.c_str());

        auto getpixel = [surf](size_t const x, size_t const y, const uint8_t c = 0) -> uint8_t
        {
            //c = 0 for red, 1 for green, 2 for blue (assuming RGB format)
            if(x >= surf->w || y >= surf->h) {return 0;}
            auto* pix = std::bit_cast<uint8_t*>(surf->pixels);
            return pix[3 * ((y) * surf -> w + x) + c];
        };

        width_ = surf->w;
        height_ = surf->h;

        altmap_.resize(width_,height_);
        termap_.resize(width_,height_);


        for(int32_t y = 0; y < height_; ++y)
        {
            for(uint32_t x = 0; x < width_; ++x)
            {
                auto const h = getpixel(x,y);
                altmap_(x,y) = h;
            }
        }

        SDL_FreeSurface(surf);

        //makeValid();

    }

    void clear()
    {
        termap_.fill(0);
        altmap_.fill(0);
    }

    void makeValid()
    {
        struct ter
        {
            uint8_t a = 0,b = 0,c = 0,d = 0;
        };

        auto selectxter = [](ter const t) -> uint8_t
        {
            if(t.a == 0 && t.b == 0 && t.c == 0 && t.d == 0)
            {
                return 0;
            }
            if(t.a == 1 && t.b == 1 && t.c == 0 && t.d == 0)
            {
                return 1;
            }
            if(t.a == 0 && t.b == 1 && t.c == 0 && t.d == 1)
            {
                return 2;
            }
            if(t.a == 0 && t.b == 0 && t.c == 1 && t.d == 1)
            {
                return 3;
            }
            if(t.a == 1 && t.b == 0 && t.c == 1 && t.d == 0)
            {
                return 4;
            }
            if(t.a == 1 && t.b == 1 && t.c == 0 && t.d == 1)
            {
                return 5;
            }
            if(t.a == 0 && t.b == 1 && t.c == 1 && t.d == 1)
            {
                return 6;
            }
            if(t.a == 1 && t.b == 0 && t.c == 1 && t.d == 1)
            {
                return 7;
            }
            if(t.a == 1 && t.b == 1 && t.c == 1 && t.d == 0)
            {
                return 8;
            }
            if(t.a == 0 && t.b == 1 && t.c == 0 && t.d == 0)
            {
                return 9;
            }
            if(t.a == 0 && t.b == 0 && t.c == 0 && t.d == 1)
            {
                return 10;
            }
            if(t.a == 0 && t.b == 0 && t.c == 1 && t.d == 0)
            {
                return 11;
            }
            if(t.a == 1 && t.b == 0 && t.c == 0 && t.d == 0)
            {
                return 12;
            }
            if(t.a == 1 && t.b == 1 && t.c == 1 && t.d == 1)
            {
                return 13;
            }
            return 0;
        };


        auto inbounds = [this](uint32_t const x, uint32_t const y) -> bool
        {
            if(x >= width_ || y >= height_) return false;
            return true;
        };
        auto gat = [&](uint32_t const x, uint32_t const y) -> uint32_t
        {
            if(inbounds(x,y)) return altmap_(x,y);
            else return 0;
        };


        uint32_t row = 0,col = 0;
        uint32_t MAX_TILE_HEIGHT = 255;
        uint32_t current_tile = 0;
        /* Top and left edge */
        for (row = 0; row < height_; row++)
        {
            for (col = 0; col < width_; col++)
            {
                current_tile = MAX_TILE_HEIGHT;
                if (col != 0)
                {
                    /* Find lowest tile; either the top or left one */
                    current_tile = altmap_(col-1,row); // top edge
                }
                if (row != 0)
                {
                    if ( altmap_( col,row - 1) < current_tile)
                    {
                        current_tile = altmap_(col,row-1); // left edge
                    }
                }

                /* Does the height differ more than one? */
                if (altmap_.at(col, row) >= current_tile + 2) {
                    /* Then change the height to be no more than one */
                    altmap_.at(col, row) = current_tile + 1;
                }
            }
        }

        /* Bottom and right edge */
        for (row = height_ - 1; row < height_; row--)
        {
            for (col = width_ - 1; col < width_; col--)
            {
                current_tile = MAX_TILE_HEIGHT;
                if (col != width_ - 1)
                {
                    /* Find lowest tile; either the bottom and right one */
                    current_tile = altmap_.at(col + 1, row); // bottom edge
                }

                if (row != height_ - 1)
                {
                    if (altmap_.at(col, row + 1) < current_tile)
                    {
                        current_tile = altmap_.at(col, row + 1); // right edge
                    }
                }

                /* Does the height differ more than one? */
                if (altmap_.at(col, row) >= current_tile + 2)
                {
                    /* Then change the height to be no more than one */
                    altmap_.at(col, row) = current_tile + 1;
                }
            }
        }




        for(uint32_t y = 0; y < height_; ++y)
        {
            for(uint32_t x = 0; x < width_; ++x)
            {

                auto const h = altmap_.at(x,y);
                ter t;
                //look up left
                if(gat(x-1,y-1) > h) { t.a = 1; }

                //look up center
                if(gat(x,y-1) > h) { t.a = 1; t.b = 1; }
                //loop up right
                if(gat(x+1,y-1) > h) { t.b = 1; }
                //left
                if(gat(x-1,y) > h) { t.a = 1; t.c = 1; }
                //right
                if(gat(x+1,y) > h) { t.b = 1; t.d = 1; }
                //down left
                if(gat(x-1,y+1) > h) { t.c = 1; }
                //down center
                if(gat(x,y+1) > h) { t.c = 1; t.d = 1; }
                //down right
                if(gat(x+1,y+1) > h) { t.d = 1; }

                auto const m = selectxter(t);
                termap_.at(x,y) = m;
            }
        }
    }


    void clipHeight(uint32_t const mh)
    {
        for(auto y = 0u; y < height_; ++y)
        {
            for(auto x = 0u; x < width_; ++x)
            {
                auto& h = altmap_.at(x,y);
                if(h > mh)
                {
                    h = mh;
                }
            }

        }
    }


    [[nodiscard]] auto getWidth() const -> uint32_t
    {
        return width_;
    }

    [[nodiscard]] auto getHeight() const -> uint32_t
    {
        return height_;
    }

    [[nodiscard]] auto termapData() -> uint32_t*
    {
        return termap_.data();
    }

    [[nodiscard]] auto altmapData() -> uint32_t*
    {
        return altmap_.data();
    }


    auto termap() -> Vector2d<uint32_t>&
    {
        return termap_;
    }

    auto altmap() -> Vector2d<uint32_t>&
    {
        return altmap_;
    }

private:
    uint32_t width_ = 0;
    uint32_t height_ = 0;

    Vector2d<uint32_t> termap_;
    Vector2d<uint32_t> altmap_;


};




}
