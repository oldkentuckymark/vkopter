#pragma once

#include <glm/vec4.hpp>

namespace vkopter::render
{

struct Material
{
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular; //specular.a = shininess
    glm::vec4 emissive;
};


}
