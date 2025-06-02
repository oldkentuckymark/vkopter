#pragma once

#include "eventwindow.hpp"

#include "grid.hpp"
#include <chrono>
#include <cstdint>
#include <random>

namespace vkopter::game::citygen
{

template<uint64_t W, uint64_t H, uint64_t D, uint64_t S>
class AtomUpdater
{
public:
   explicit AtomUpdater(Grid<W,H,D>& g) :
       grid_(g),
       dis_percent_(0,100),
       dis_ew_(-EventWindow<S>::SIZE,EventWindow<S>::SIZE),
       dis_grid_x_(0,grid_.WIDTH-1),
       dis_grid_y_(0,grid_.HEIGHT-1),
       dis_grid_z_(0,grid_.DEPTH-1),
       dis_bool_(0,1),
       funcs_(get_funcs()),
       gen_(87735410)
   {
   }

   AtomUpdater(AtomUpdater&) = default;
   auto operator=(AtomUpdater&) -> AtomUpdater& = default;
   AtomUpdater(AtomUpdater&&) = default;
   auto operator=(AtomUpdater&&) -> AtomUpdater& = default;
   ~AtomUpdater() = default;

   //explicit AtomUpdater(AtomUpdater const & that) = default;


   auto update(int32_t const x, int32_t const y, int32_t const z = 0) -> void
   {
       if(grid_(x,y,z).type)
       {
           return;//copy EW is broken!!!!
           EventWindow<S> ew = grid_.template copyEW<S>(x,y,z);

           //int db = grid_(x,y,z).type;
           (this->*(funcs_[grid_(x,y,z).type])) (ew);
           grid_.template pasteEW<S>(ew,x,y,z);

       }

       //std::cout << "update from thread: " << std::this_thread::get_id() << "\n";
   }

   auto updateRnd() -> void
   {
       update( rndGridX(),rndGridY(),rndGridZ() );
   }


   auto seedRNG(unsigned long s = 0) -> void
   {
       if(s == 0)
       {
           s = std::chrono::high_resolution_clock::now().time_since_epoch().count();
       }
       gen_.seed(s);
   }

   auto isOverlaping(int32_t const x1, int32_t const y1, int32_t const z1,
                     int32_t const x2, int32_t const y2, int32_t const z2) -> bool
   {
       return (x1 + S >= x2 - S) &&
            (x1 - S <= x2 + S) &&
            (y1 + S >= y2 - S) &&
            (y1 - S <= y2 + S) &&
            (z1 + S >= z2 - S) &&
            (z1 - S <= z2 + S);
   };




private:

    Grid<W,H,D>& grid_;

   std::minstd_rand gen_;
   std::uniform_int_distribution<int> dis_bool_;
   std::uniform_int_distribution<int> dis_percent_;
   std::uniform_int_distribution<int> dis_ew_;
   std::uniform_int_distribution<int> dis_grid_x_;
   std::uniform_int_distribution<int> dis_grid_y_;
   std::uniform_int_distribution<int> dis_grid_z_;

   std::array< void (AtomUpdater::*)(EventWindow<S>&), NUM_TYPES > funcs_;


////////////////////////////////////////////////////////////

   //consts for CA
   int const static INTERSECTION_CHANCE = 7;

   auto coin() -> bool
   {
       return dis_bool_(gen_);
   }

   auto rndPercent(const int p) -> bool
   {
       const int n = dis_percent_(gen_);
       return n <= p;
   }

   auto rndNum(const int a, const int b) -> int
   {
       std::uniform_int_distribution<int> d(a,b);
       return d(gen_);
   }

   auto rndEW() -> int32_t
   {
       return dis_ew_(gen_);
   }

   auto rndGridX() -> int32_t
   {
       return dis_grid_x_(gen_);
   }

   auto rndGridY() -> int32_t
   {
       return dis_grid_y_(gen_);
   }

   auto rndGridZ() -> int32_t
   {
       return dis_grid_z_(gen_);
   }


   auto f00(EventWindow<S>& ew) -> void
   {

   }
   auto f01(EventWindow<S>& ew) -> void
   {

   }
   auto f02(EventWindow<S>& ew) -> void
   {
       return;
       //ew.siteMemory(0,0,0,0);
       //first thing, fix road, as it might not actually be what it says RN
       bool fixed = false;

       auto EWtypes = {RoadEW, RoadEWN, RoadEWS, Road4Way};
       if(ew(1,0) == EWtypes)
       {

       }

       if(fixed) {return;}

       int cx = 0, cy = 1;
       bool const lookNorth = coin();
       if(lookNorth) { cy = -1; }
       bool const inter = rndPercent(INTERSECTION_CHANCE);

       auto possibleTypes = {RoadNS};

       if(lookNorth && inter)
       {
           possibleTypes = {RoadDiagBL, RoadDiagBR, Road4Way, RoadEWS, RoadNSE, RoadNSW};

       }
       if(!lookNorth && inter)
       {
           possibleTypes = {RoadDiagTL, RoadDiagTR, Road4Way, RoadEWN, RoadNSE, RoadNSW};
       }
       //ew(cx,cy) << possibleTypes;

   }
   auto f03(EventWindow<S>& ew) -> void
   {
       return;
       int cx = 1, cy = 0;
       bool const lookWest = coin();
       if(lookWest) { cx = -1; }
       bool const inter = rndPercent(INTERSECTION_CHANCE);

       auto possibleTypes = {RoadEW};

       if(lookWest && inter)
       {
           possibleTypes = {RoadDiagBR, RoadDiagTR, Road4Way, RoadEWN, RoadEWS, RoadNSE};
       }
       if(!lookWest && inter)
       {
           possibleTypes = {RoadDiagBL, RoadDiagTL, Road4Way, RoadEWN, RoadEWS, RoadNSW};
       }
       ew(cx,cy) << possibleTypes;
   }
   auto f04(EventWindow<S>& ew) -> void
   {
       //look south or west
       int cx = -1, cy = 0;
       bool const lookSouth = coin();
       if(lookSouth) { cx = 0; cy = 1; }

       bool const inter = rndPercent(INTERSECTION_CHANCE);
       if(lookSouth)
       {
           if(inter)
           {

               ew(cx,cy) << RoadNS;
           }
           else
           {
               ew(cx,cy) << RoadDiagTR;
           }
       }
       else
       {
           if(inter)
           {

               ew(cx,cy) << RoadEW;
           }
           else
           {
               ew(cx,cy) << RoadDiagTR;
           }
       }

   }
   auto f05(EventWindow<S>& ew) -> void
   {
       //look south or east
       int cx=1,cy = 0;
       bool const lookSouth = coin();
       if(lookSouth) { cx = 0; cy = 1; }

       bool const inter = rndPercent(INTERSECTION_CHANCE);
       if(lookSouth)
       {
           if(inter)
           {

               ew(cx,cy) << RoadNS;
           }
           else
           {
               ew(cx,cy) << RoadDiagTL;
           }
       }
       else
       {
           if(inter)
           {

               ew(cx,cy) << RoadEW;
           }
           else
           {
               ew(cx,cy) << RoadDiagTL;
           }
       }
   }
   auto f06(EventWindow<S>& ew) -> void
   {
       //look north or east
       int cx=1,cy=0;
       bool const lookNorth = coin();
       if(lookNorth) { cx = 0; cy = -1; }

       bool const inter = rndPercent(INTERSECTION_CHANCE);
       if(lookNorth)
       {
           if(inter)
           {

               ew(cx,cy) << RoadNS;
           }
           else
           {
               ew(cx,cy) << RoadDiagBL;
           }
       }
       else
       {
           if(inter)
           {

               ew(cx,cy) << RoadEW;
           }
           else
           {
               ew(cx,cy) << RoadDiagBL;
           }
       }
   }
   auto f07(EventWindow<S>& ew) -> void
   {
       //look north or west
       int cx=-1,cy=0;
       bool const lookNorth = coin();
       if(lookNorth) { cx = 0; cy = -1; }

       bool const inter = rndPercent(INTERSECTION_CHANCE);
       if(lookNorth)
       {
           if(inter)
           {

               ew(cx,cy) << RoadNS;
           }
           else
           {
               ew(cx,cy) << RoadDiagBR;
           }
       }
       else
       {
           if(inter)
           {

               ew(cx,cy) << RoadEW;
           }
           else
           {
               ew(cx,cy) << RoadDiagBR;
           }
       }
   }
   auto f08(EventWindow<S>& ew) -> void
   {
       ew(-1,0) << RoadEW;
       ew(1,0) << RoadEW;
       ew(0,1) << RoadNS;
       ew(0,-1) << RoadNS;
   }
   auto f09(EventWindow<S>& ew) -> void
   {
       ew(1,0) << RoadEW;
       ew(-1,0) << RoadEW;
       ew(0,-1) << RoadNS;
   }
   auto f10(EventWindow<S>& ew) -> void
   {
       ew(1,0) << RoadEW;
       ew(-1,0) << RoadEW;
       ew(0,1) << RoadNS;
   }
   auto f11(EventWindow<S>& ew) -> void
   {
       ew(0,-1) << RoadNS;
       ew(0,1) << RoadNS;
       ew(1,0) << RoadEW;
   }
   auto f12(EventWindow<S>& ew) -> void
   {
       ew(0,-1) << RoadNS;
       ew(0,1) << RoadNS;
       ew(-1,0) << RoadEW;
   }

//////////////////////////////////////////////////////////////////


   static constexpr auto get_funcs() -> std::array<void (AtomUpdater<W,H,D,S>::*)(EventWindow<S> &), NUM_TYPES>
   {
           return
           {
               &AtomUpdater::f00,&AtomUpdater::f01,&AtomUpdater::f02,&AtomUpdater::f03,
               &AtomUpdater::f04,&AtomUpdater::f05,&AtomUpdater::f06,&AtomUpdater::f07,
               &AtomUpdater::f08,&AtomUpdater::f09,&AtomUpdater::f10,&AtomUpdater::f11,
               &AtomUpdater::f12
           };
   }


};

}

