#pragma once


#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform2.hpp>

namespace vkopter::game
{

class Camera
{
public:
    Camera()
    {
        //lookAt({0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f},{0.0f,1.0f,0.0f});
        update();
    }

    auto setPerspective(float const fov = 45.0f, float const width = 1280, float const height = 720, float const near = 0.0001f, float const far = 10000.0f) -> void
    {
        fov_ = fov;
        width_ = width;
        height_ = height;
        near_ = near;
        far_ = far;
        update();
    }

    [[nodiscard]] auto getView() -> glm::mat4
    {
        update();
        return view_;
    }

    [[nodiscard]] auto getProjection() -> glm::mat4
    {
        update();
        return proj_;
    }

private:
    auto update() -> void
    {
        //view_ = glm::lookAt( glm::vec3{0.0f,0.0f,420.0f}, {0.0f,0.0f,0.0f}, {0.0f,1.0f,0.0f});
        view_ = glm::mat4(1.0f);
        view_ = glm::translate(view_, glm::vec3{0.0f,2.0f,4.0f});
        proj_ = glm::perspectiveLH_ZO(fov_,
                                         width_/
                                         height_,
                                         near_,
                                         far_);
        //proj_ = glm::rotate(proj_, 10.0f,glm::vec3(1.0f,0.0f,0.0f) );
        //proj_ = glm::scale(proj_,{1.0f,1.0f,1.0f});
        view_ = glm::lookAtLH(glm::vec3{25.0f,-10.0f,-30.0f},glm::vec3{25.0f,0.0f,0.0f},glm::vec3{0.0f,1.0f,0.0f});

    }
public:
    glm::mat4 view_;
    glm::mat4 proj_;
    glm::vec3 position_ = {0.0f,0.0f,0.0f}; float padding0;
    glm::vec3 dir_ = {0.0f,0.0f,1.0f}; float padding1;
    glm::vec3 up_ = {0.0f,1.0f,0.0f}; float padding2;
    glm::vec3 velocity_ = {0.0f,0.0f,0.0f}; float padding3;
    float width_ = 1280.0f;
    float height_ = 720.0f;
    float fov_ = 45.0f;
    float near_ = 0.0001f;
    float far_ = 10000.0f;
    uint32_t padding[11];

};

}

