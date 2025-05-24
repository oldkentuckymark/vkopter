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

    Mesh() = default;
    ~Mesh() = default;
    Mesh(Mesh const &) = default;
    auto operator = (Mesh const &) -> Mesh& = default;
    auto operator = (Mesh &&) -> Mesh& = default;

    explicit Mesh(std::string const & path)
    {
        if(path.ends_with(".gltf") || true)
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
            glm::vec3  pos = pPos[i];
            glm::vec2  tex = pTex[i];
            glm::vec3  norm = pNorm[i];

            float t = pos.y;
            pos.y = -pos.z;
            pos.z = t;

            t = norm.y;
            norm.y = -norm.z;
            norm.z = t;

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

}
