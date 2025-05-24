#pragma once

#include <cstdint>

#include "../render/renderobject.hpp"
#include <glm/glm.hpp>


namespace vkopter::game
{

struct Entity
{
public:

private:
    render::RenderObject render_object_;
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;


};

}
