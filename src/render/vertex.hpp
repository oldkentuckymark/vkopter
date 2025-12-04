#pragma once

#include <cstddef>
#include <glm/vec4.hpp>
#include <vulkan/vulkan.hpp>
#include <array>

namespace vkopter::render
{

struct Vertex
{
    glm::vec4 position;
    glm::vec4 tex;
    glm::vec4 normal;

    std::array<vk::VertexInputBindingDescription,1> static constexpr  vertexInputBindingDescriptions =
    {
        vk::VertexInputBindingDescription{0,sizeof(glm::vec4) * 3,vk::VertexInputRate::eVertex}
    };

    std::array<vk::VertexInputAttributeDescription,3> static constexpr vertexInputAttribureDescriptions =
    {
        vk::VertexInputAttributeDescription{0,0,vk::Format::eR32G32B32A32Sfloat,0},
        vk::VertexInputAttributeDescription{1,0,vk::Format::eR32G32B32A32Sfloat,16},
        vk::VertexInputAttributeDescription{2,0,vk::Format::eR32G32B32A32Sfloat,32}
    };

};

}
