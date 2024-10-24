#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RIGHT_HANDED
#define VMA_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL

#include <cstdint>

#include "render/vk_mem_alloc.h"
#include <vulkan/vulkan.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "render/renderobject.hpp"
#include "render/vulkanrenderer.hpp"
#include "vulkanwindow.hpp"

#include "glm/ext/matrix_transform.hpp"

#include "game/terrain.hpp"

#include "Jolt/Jolt.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "util/tiny_gltf.hpp"

auto main(int argc, char **argv) -> int
{
    vkopter::VulkanWindow window(1280, 720);
    vkopter::render::VulkanRenderer renderer(window, window.getMemoryManager());

    vkopter::game::Terrain terrain;
    terrain.load("data/maps/test/hm.bmp");
    terrain.makeValid();
    renderer.resizeTerrain(terrain.getWidth(), terrain.getHeight());
    renderer.updateTerrain(terrain.termap_.data(), terrain.altmap_.data());
    //terrain.clear();

    auto mesh0 = renderer.createMesh("data/meshes/monkey.gltf");
    auto mat0 = renderer.createMaterial();
    auto matx0 = renderer.createModelMatrix();
    auto cam0 = renderer.createCamera();
    auto light0 = renderer.createLight();

    auto &mat0ref = renderer.getMaterialRef(mat0);
    mat0ref.ambient = {1.0f, 1.0f, 1.0f, 1.0f};
    mat0ref.diffuse = {1.0f, 0.0f, 0.0f, 1.0f};

    auto &matx0ref = renderer.getModelMatrixRef(matx0);
    matx0ref = glm::translate(glm::mat4(1.0f), {2.0f, 10.0f, 10.0f});

    auto &cam0ref = renderer.getCameraRef(cam0);

    auto &light0ref = renderer.getLightRef(light0);
    light0ref.ambient = {0.3f, 0.3f, 0.3f, 1.0f};
    light0ref.diffuse = {0.3f, 0.3f, 0.3f, 1.0f};
    light0ref.position = {0.0f, 1000.0f, 0.0f, 1.0f};
    light0ref.direction = {1.0f, -10.0f, 0.0f, 1.0f};

    auto mesh1 = renderer.createMesh("data/meshes/monkey.gltf");
    auto mat1 = renderer.createMaterial();
    auto matx1 = renderer.createModelMatrix();
    auto cam1 = renderer.createCamera();
    auto light1 = renderer.createLight();

    auto &mat1ref = renderer.getMaterialRef(mat1);
    mat1ref.ambient = {1.0f, 1.0f, 1.0f, 1.0f};
    mat1ref.diffuse = {1.0f, 0.0f, 1.0f, 1.0f};

    auto &matx1ref = renderer.getModelMatrixRef(matx1);
    matx1ref = glm::translate(glm::mat4(1.0f), {-6.0f, 0.0f, 0.0f});

    auto &cam1ref = renderer.getCameraRef(cam1);

    auto &light1ref = renderer.getLightRef(light1);
    light1ref.ambient = {0.1f, 0.1f, 0.1f, 1.0f};
    light1ref.diffuse = {1.0f, 1.0f, 1.0f, 1.0f};

    auto mat2 = renderer.createMaterial();
    auto matx2 = renderer.createModelMatrix();

    auto &mat2ref = renderer.getMaterialRef(mat2);
    mat2ref.ambient = {0.1, 0.1, 0.1, 1.0};
    mat2ref.diffuse = {0.0f, 1.0f, 0.0f, 1.0f};

    auto &matx2ref = renderer.getModelMatrixRef(matx2);
    matx2ref = glm::translate(glm::mat4(1.0f), {-4.0f, -4.0f, 0.0f});

    vkopter::render::RenderObject ro0{mesh0, mat0, cam0, matx0};
    vkopter::render::RenderObject ro1{mesh1, mat1, cam0, matx1};
    vkopter::render::RenderObject ro2{mesh1, mat2, cam0, matx2};

    auto ro0ref = renderer.createRenderObject(ro0);
    auto ro1ref = renderer.createRenderObject(ro1);
    auto ro2ref = renderer.createRenderObject(ro2);

    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            } else if (e.type == SDL_WINDOWEVENT) {
                if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    window.recreate_swapchain();
                    renderer.releaseSwapChainResources();
                    renderer.initSwapChainResources();
                    //cam0ref.setPerspective(45.0f,window.getWidth(),window.getHeight());
                }
            }
        }

        uint8_t const *const keys = SDL_GetKeyboardState(nullptr);
        if (keys[SDL_SCANCODE_ESCAPE]) {
            running = false;
        }
        if (keys[SDL_SCANCODE_W]) {
            cam0ref.moveForward(0.1f);
        }
        if (keys[SDL_SCANCODE_S]) {
            cam0ref.moveForward(-0.1f);
        }
        if (keys[SDL_SCANCODE_A]) {
            cam0ref.moveRight(-0.1f);
        }
        if (keys[SDL_SCANCODE_D]) {
            cam0ref.moveRight(0.1f);
        }
        if (keys[SDL_SCANCODE_Q]) {
            cam0ref.addRoll(0.1f);
        }
        if (keys[SDL_SCANCODE_E]) {
            cam0ref.addRoll(-0.1f);
        }
        if (keys[SDL_SCANCODE_R]) {
            cam0ref.moveUp(0.1f);
        }
        if (keys[SDL_SCANCODE_F]) {
            cam0ref.moveUp(-0.1f);
        }
        if (keys[SDL_SCANCODE_T]) {
            cam0ref.floatUp(0.1f);
        }
        if (keys[SDL_SCANCODE_G]) {
            cam0ref.floatUp(-0.1f);
        }
        if (keys[SDL_SCANCODE_UP]) {
            cam0ref.addPitch(0.1f);
        }
        if (keys[SDL_SCANCODE_DOWN]) {
            cam0ref.addPitch(-0.1f);
        }
        if (keys[SDL_SCANCODE_RIGHT]) {
            cam0ref.panRight(0.1f);
        }
        if (keys[SDL_SCANCODE_LEFT]) {
            cam0ref.panRight(-0.1f);
        }
        if (keys[SDL_SCANCODE_Z]) {
            cam0ref.addYaw(0.1f);
        }
        if (keys[SDL_SCANCODE_X]) {
            cam0ref.addYaw(-0.1f);
        }

        renderer.startNextFrame();
    }

    renderer.removeRenderObject(ro0ref);
    renderer.removeRenderObject(ro1ref);
    renderer.removeRenderObject(ro2ref);

    return 0;
}
