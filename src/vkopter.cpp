#define VMA_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "util/tiny_gltf.hpp"
#undef TINYGLTF_IMPLEMENTATION
#undef STB_IMAGE_IMPLEMENTATION
#undef STB_IMAGE_WRITE_IMPLEMENTATION


#include <cstdint>

#include "render/vk_mem_alloc.h"
#include <vulkan/vulkan.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "render/renderobject.hpp"
#include "render/vulkanrenderer.hpp"
#include "vulkanwindow.hpp"
#include "game/citygen/grid.hpp"
#include "game/citygen/atomupdater.hpp"

#include <glm/ext/matrix_transform.hpp>

#include "game/terrain.hpp"

#include "util/stb_image.h"


auto main(int argc, char **argv) -> int
{
    vkopter::VulkanWindow window(1280, 720);
    vkopter::render::VulkanRenderer renderer(window, window.getMemoryManager());

    vkopter::game::citygen::Grid<256, 256, 1> grid;
    vkopter::game::citygen::AtomUpdater<256, 256, 1, 4> au(grid);
    grid.clear();
    grid(16, 16, 0) = vkopter::game::citygen::RoadNS;
    for(auto i = 0ul; i < 1000000; ++i)
    {
        au.updateRnd();
    }


    vkopter::game::Terrain terrain;
    terrain.load("data/maps/test/hm.bmp");
    terrain.makeValid();
    renderer.resizeTerrain(terrain.getWidth(), terrain.getHeight());
    renderer.updateTerrain(terrain.termapData(), terrain.altmapData());
    renderer.updateGrid<grid.WIDTH,grid.HEIGHT,grid.DEPTH>(grid.atoms());

    auto mesh0 = renderer.createMesh("data/meshes/untitled.gltf");
    auto mat0 = renderer.createMaterial();
    auto matx0 = renderer.createModelMatrix();
    auto cam0 = renderer.createCamera();
    auto light0 = renderer.createLight();

    auto &mat0ref = renderer.getMaterialRef(mat0);
    mat0ref.ambient = {1.0f, 1.0f, 1.0f, 1.0f};
    mat0ref.diffuse = {1.0f, 0.0f, 0.0f, 1.0f};

    auto &matx0ref = renderer.getModelMatrixRef(matx0);
    matx0ref = glm::translate(glm::mat4(1.0f), {0.0f, -4.0f, 0.0f});

    auto &cam0ref = renderer.getCameraRef(cam0);

    auto &light0ref = renderer.getLightRef(light0);
    light0ref.ambient = {0.3f, 0.3f, 0.3f, 1.0f};
    light0ref.diffuse = {0.3f, 0.3f, 0.3f, 1.0f};
    light0ref.position = {0.0f, 1000.0f, 0.0f, 1.0f};
    light0ref.direction = {1.0f, -10.0f, 0.0f, 1.0f};

    vkopter::render::RenderObject ro0{mesh0, mat0, cam0, matx0};


    auto ro0ref = renderer.createRenderObject(ro0);


    SDL_SetRelativeMouseMode(SDL_TRUE);
    bool running = true;
    while (running)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT)
            {
                running = false;
            }
            else if(e.type == SDL_WINDOWEVENT)
            {
                if(e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    window.recreateSwapchain();
                    renderer.releaseSwapChainResources();
                    renderer.initSwapChainResources();
                    //cam0ref.setPerspective(45.0f,window.getWidth(),window.getHeight());
                }
            }
            else if(e.type == SDL_MOUSEMOTION)
            {
                cam0ref.yaw_ -= (float)e.motion.xrel / 200.f;
                cam0ref.pitch_ -= (float)e.motion.yrel / 200.f;
            }
        }

        uint8_t const * const keys = SDL_GetKeyboardState(nullptr);
        if (keys[SDL_SCANCODE_ESCAPE])
        {
            running = false;
        }
        if (keys[SDL_SCANCODE_W])
        {
            cam0ref.move({0.0f,0.0f,0.1f});
        }
        if (keys[SDL_SCANCODE_S])
        {
            cam0ref.move({0.0f,0.0f,-0.1f});
        }
        if (keys[SDL_SCANCODE_A])
        {
            cam0ref.move({-1.0f,0.0f,0.0f});
        }
        if (keys[SDL_SCANCODE_D])
        {
            cam0ref.move({1.0f,0.0f,0.0f});
        }
        if (keys[SDL_SCANCODE_R])
        {
            cam0ref.move({0.0f,-1.0f,0.0f});
        }
        if (keys[SDL_SCANCODE_F])
        {
            cam0ref.move({0.0f,1.0f,0.0f});
        }

        for(auto i = 0ul; i < 1000; ++i)
        {
            au.updateRnd();
        }

        cam0ref.update();

        renderer.updateGrid<grid.WIDTH,grid.HEIGHT,grid.DEPTH>(grid.atoms());
        renderer.startNextFrame();

    }

    renderer.removeRenderObject(ro0ref);
    //renderer.removeRenderObject(ro1ref);
    //renderer.removeRenderObject(ro2ref);

    return 0;
}
