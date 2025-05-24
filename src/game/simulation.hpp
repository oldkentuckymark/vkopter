 #pragma once

#include <cstdint>
#include "entt/entt.hpp"

namespace vkopter::game
{

class Simulation
{
public:

    Simulation()
    {
        auto l = registry.create();
    }

    ~Simulation()
    {

    }


    auto update(double const dt) -> void
    {

    }


private:
    entt::registry registry;
    std::vector<entt::entity> entities;








};


}
