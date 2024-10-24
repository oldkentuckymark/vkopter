#pragma once

#include <bit>
#include <cstdint>
#include <vector>
#include <glm/glm.hpp>
#include "util/tiny_gltf.hpp"
#include <vulkan/vulkan.hpp>
#include <SDL2/SDL_surface.h>

namespace vkopter::render
{

class Mesh
{
public:
    std::array<vk::VertexInputBindingDescription,3> static constexpr  vertexInputBindingDescriptions =
    {
        vk::VertexInputBindingDescription{0,sizeof(glm::vec4),vk::VertexInputRate::eVertex},
        vk::VertexInputBindingDescription{1,sizeof(glm::vec4),vk::VertexInputRate::eVertex},
        vk::VertexInputBindingDescription{2,sizeof(glm::vec4),vk::VertexInputRate::eVertex}
    };

    std::array<vk::VertexInputAttributeDescription,3> static constexpr vertexInputAttribureDescriptions =
    {
        vk::VertexInputAttributeDescription{0,0,vk::Format::eR32G32B32A32Sfloat,0},
        vk::VertexInputAttributeDescription{1,1,vk::Format::eR32G32B32A32Sfloat,0},
        vk::VertexInputAttributeDescription{2,2,vk::Format::eR32G32B32A32Sfloat,0}
    };

    Mesh() = default;
    ~Mesh() = default;
    Mesh(Mesh const &) = default;
    auto operator = (Mesh const &) -> Mesh& = default;
    auto operator = (Mesh &&) -> Mesh& = default;

    explicit Mesh(std::string const & path)
    {
        if(path.ends_with(".gltf"))
        {
            load_gltf(path);
        }
        else
        {
            std::abort();
        }
    }

    [[nodiscard]] auto getPositions() const -> std::vector<glm::vec4> const &
    {
        return positions_;
    }

    [[nodiscard]] auto getTexCoords() const -> std::vector<glm::vec4> const &
    {
        return texcoords_;
    }

    [[nodiscard]] auto getNormals() const -> std::vector<glm::vec4> const &
    {
        return normals_;
    }

    [[nodiscard]] auto getIndicies() const -> std::vector<uint32_t> const &
    {
        return indicies_;
    }

private:
    void load_gltf(std::string const & path)
    {
        tinygltf::TinyGLTF loader;
        tinygltf::Model model;
        std::string err;
        std::string warn;

        bool r = loader.LoadASCIIFromFile(&model,&err,&warn,path);

        uint32_t numVerts = model.accessors[model.meshes[0].primitives[0].attributes.at("POSITION")].count;
        uint32_t numIndices = model.accessors[model.meshes[0].primitives[0].indices].count;

        //positions
        auto posAcc = model.meshes[0].primitives[0].attributes.at("POSITION");
        auto positionOffset = model.bufferViews[model.accessors[posAcc].bufferView].byteOffset;
        auto positionLength = model.bufferViews[model.accessors[posAcc].bufferView].byteLength;
        auto positionData = model.buffers[model.bufferViews[model.accessors[posAcc].bufferView].buffer].data.data() + positionOffset;


        //tex
        auto texAcc = model.meshes[0].primitives[0].attributes.at("TEXCOORD_0");
        auto texOffset = model.bufferViews[model.accessors[texAcc].bufferView].byteOffset;
        auto texLength = model.bufferViews[model.accessors[texAcc].bufferView].byteLength;
        auto texData = model.buffers[model.bufferViews[model.accessors[texAcc].bufferView].buffer].data.data() + texOffset;


        //norm
        auto normAcc = model.meshes[0].primitives[0].attributes.at("NORMAL");
        auto normOffset = model.bufferViews[model.accessors[normAcc].bufferView].byteOffset;
        auto normLength = model.bufferViews[model.accessors[normAcc].bufferView].byteLength;
        auto normData = model.buffers[model.bufferViews[model.accessors[normAcc].bufferView].buffer].data.data() + normOffset;


        //index
        auto indexAcc = model.meshes[0].primitives[0].indices;
        auto indexOffset = model.bufferViews[model.accessors[indexAcc].bufferView].byteOffset;
        auto indexLength = model.bufferViews[model.accessors[indexAcc].bufferView].byteLength;
        auto indexData = model.buffers[model.bufferViews[model.accessors[indexAcc].bufferView].buffer].data.data() + indexOffset;



        auto* pPos = std::bit_cast<glm::vec3*>(positionData);
        auto* pTex = std::bit_cast<glm::vec2*>(texData);
        auto* pNorm = std::bit_cast<glm::vec3*>(normData);
        auto* pIdx = std::bit_cast<uint16_t*>(indexData);

        for(auto i = 0ul; i < numVerts; ++i)
        {
            glm::vec3 const pos = pPos[i];
            glm::vec2 const tex = pTex[i];
            glm::vec3 const norm = pNorm[i];

            positions_.emplace_back(pos,1.0f);
            texcoords_.emplace_back(tex,0.0f,0.0f);
            normals_.emplace_back(norm,1.0f);
        }

        for(auto i = 0ul; i < numIndices; ++i)
        {
            uint16_t const idx = pIdx[i];
            indicies_.push_back(idx);
        }
    }


    std::vector<glm::vec4> positions_;
    std::vector<glm::vec4> texcoords_;
    std::vector<glm::vec4> normals_;
    std::vector<uint32_t> indicies_;
};

class WaterMesh
{
public:

    std::array<vk::VertexInputBindingDescription,3> static constexpr  vertexInputBindingDescriptions =
    {
        vk::VertexInputBindingDescription{0,sizeof(glm::vec4),vk::VertexInputRate::eVertex},
        vk::VertexInputBindingDescription{1,sizeof(glm::vec4),vk::VertexInputRate::eVertex},
        vk::VertexInputBindingDescription{2,sizeof(glm::vec4),vk::VertexInputRate::eVertex}
    };

    std::array<vk::VertexInputAttributeDescription,3> static constexpr vertexInputAttribureDescriptions =
    {
        vk::VertexInputAttributeDescription{0,0,vk::Format::eR32G32B32A32Sfloat,0},
        vk::VertexInputAttributeDescription{1,1,vk::Format::eR32G32B32A32Sfloat,0},
        vk::VertexInputAttributeDescription{2,2,vk::Format::eR32G32B32A32Sfloat,0}
    };

    explicit WaterMesh(int const waterSize = 256.0f, int const divisions = 8.0f)
    {
        //init water mesh
        auto const bs = static_cast<float>(waterSize / divisions);
        for(int x = 0; x < divisions; ++x)
        {
            for(int z = 0; z < divisions; ++z)
            {
                float const xc = static_cast<float>(x)*bs;
                float const zc = static_cast<float>(z)*bs;
                positions_.emplace_back(xc, 0.0f, zc, 1.0f);
                positions_.emplace_back(xc, 0.0f, zc+bs, 1.0f);
                positions_.emplace_back(xc+bs, 0.0f, zc+bs, 1.0f);
                positions_.emplace_back(xc+bs, 0.0f, zc, 1.0f);

                normals_.emplace_back(0.0f,-1.0f,0.0f,0.0f);
                normals_.emplace_back(0.0f,-1.0f,0.0f,0.0f);
                normals_.emplace_back(0.0f,-1.0f,0.0f,0.0f);
                normals_.emplace_back(0.0f,-1.0f,0.0f,0.0f);

                texcoords_.emplace_back(0.0f,0.0f,0.0f,0.0f);
                texcoords_.emplace_back(0.0f,0.0f,0.0f,0.0f);
                texcoords_.emplace_back(0.0f,0.0f,0.0f,0.0f);
                texcoords_.emplace_back(0.0f,0.0f,0.0f,0.0f);
            }
        }


        for(int i = 0; i < positions_.size(); ++i)
        {
            indicies_.push_back(i);
        }
    }

    [[nodiscard]] auto getPositions() const -> std::vector<glm::vec4> const &
    {
        return positions_;
    }

    [[nodiscard]] auto getTexCoords() const -> std::vector<glm::vec4> const &
    {
        return texcoords_;
    }

    [[nodiscard]] auto getNormals() const -> std::vector<glm::vec4> const &
    {
        return normals_;
    }

    [[nodiscard]] auto getIndicies() const -> std::vector<uint32_t> const &
    {
        return indicies_;
    }

private:
    std::vector<glm::vec4> positions_;
    std::vector<glm::vec4> texcoords_;
    std::vector<glm::vec4> normals_;
    std::vector<uint32_t> indicies_;
};

}
