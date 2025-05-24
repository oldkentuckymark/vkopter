#pragma once

#include "glm/vec4.hpp"

namespace vkopter::render
{

class Light
{
public:
    void setAmbient(glm::vec4 v){ambient = v;}
    void setDiffuse(glm::vec4 v){diffuse = v;}
    void setSpecular(glm::vec4 v){specular = v;}
    void setPosition(glm::vec4 v){position = v;}
    void setDirection(glm::vec4 v){direction = v;}
    void setConstantAttenuation(float const a){attenuation.x = a;}
    void setLinearAttenuation(float const a){attenuation.y = a;}
    void setQuadraticAttenuation(float const a){attenuation.z = a;}
    void setCutoffAngle(float const a){attenuation.w = a;}


    glm::vec4 ambient = {};
    glm::vec4 diffuse = {};
    glm::vec4 specular = {};

    glm::vec4 position = {};
    glm::vec4 direction = {};

    //{constatt,linatt,quadatt,cutoffangle}
    glm::vec4 attenuation = {};

};

}
