#pragma once


#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace vkopter::game
{

class Camera
{
public:
    Camera()
    {
        update();
    }

    auto addPitch(float const s) -> void
    {
        glm::vec3 const rightVector = glm::cross(dir_,up_);
        dir_ = glm::rotate(dir_,-s,rightVector);
        update();
    }

    auto addYaw(float const s) -> void
    {
        dir_ = glm::rotate(dir_,-s,up_);
        update();
    }

    auto addRoll(float const s) -> void
    {
        up_ = glm::rotate(up_, -s, {0.0f,0.0f,1.0f});
        update();
    }

    auto moveForward(float const s) -> void
    {
        position_ += (dir_ * s);
        update();
    }

    auto moveRight(float const s) -> void
    {
        glm::vec3 const rightVector = glm::cross(dir_,up_);
        position_ -= (rightVector * s);
        update();
    }

    auto moveUp(float const s) -> void
    {
        position_ -= (up_ * s);
        update();
    }

    auto panRight(float const s) -> void
    {
        dir_ = glm::rotate(dir_,-s,{0.0f,-1.0f,0.0f});
        update();
    }

    auto floatUp(float const s) -> void
    {
        position_ += (glm::vec3{0.0f,-1.0f,0.0f}*s);
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


    auto lookAt(glm::vec3 position, glm::vec3 target, glm::vec3 up) -> void
    {
        position_ = position;
        dir_ = glm::normalize(target - position);
        up_ = up;
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
        view_ = glm::lookAt(position_,position_ + dir_,up_);
        proj_ = glm::perspectiveRH_ZO(fov_,
                                         width_/
                                         height_,
                                         near_,
                                         far_);
        proj_ = glm::scale(proj_,{-0.01f,0.01f,0.01f});
        //proj_ = glm::scale(proj_, {1.0f,});
    }

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

