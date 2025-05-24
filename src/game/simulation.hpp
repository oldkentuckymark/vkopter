 #pragma once

#include "entt/entt.hpp"

namespace vkopter::game
{

class Simulation
{
public:

    Simulation()
    {
        auto l = registry_.create();
    }

    ~Simulation()
    {

    }


    auto update(double const dt) -> void
    {

    }


private:
    entt::registry registry_;
    std::vector<entt::entity> entities_;








};


}
