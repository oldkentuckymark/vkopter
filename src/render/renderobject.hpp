#pragma once

#include <cstdint>

namespace vkopter::render
{

struct RenderObject
{
    uint32_t mesh;
    uint32_t material;
    uint32_t camera;
    uint32_t matrix;

    auto operator==(RenderObject const& that) const -> bool
    {
        return  mesh == that.mesh &&
                material == that.material &&
                camera == that.camera &&
                matrix == that.matrix;
    }
};


}
