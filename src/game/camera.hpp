#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform2.hpp>


//shamelessly stolen from vkguide.dev don't @ me

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

    auto move(glm::vec3 const & v ) -> void
    {
        velocity_ += v;
    }

    auto update() -> void
    {
        glm::mat4 cameraRotation = get_rotation_matrix();
        position_ += glm::vec3(cameraRotation * glm::vec4(velocity_ * 0.5f, 0.f));
        proj_ = glm::perspectiveLH_ZO(fov_,
                                      width_/
                                          height_,
                                      near_,
                                      far_);
        proj_ = glm::scale(proj_, {0.01f,0.01f,0.01f});
        view_ = get_view_matrix();
        velocity_ = {0.0f,0.0f,0.0f};
    }

private:

    auto get_view_matrix() -> glm::mat4
    {
        // to create a correct model view, we need to move the world in opposite
        // direction to the camera
        //  so we will create the camera model matrix and invert
        glm::mat4 cameraTranslation = glm::translate(glm::mat4(1.f), position_);
        glm::mat4 cameraRotation = get_rotation_matrix();
        return glm::inverse(cameraTranslation * cameraRotation);
    }

    auto get_rotation_matrix() -> glm::mat4
    {
        // fairly typical FPS style camera. we join the pitch and yaw rotations into
        // the final rotation matrix

        glm::quat pitchRotation = glm::angleAxis(pitch_, glm::vec3 { 1.f, 0.f, 0.f });
        glm::quat yawRotation = glm::angleAxis(yaw_, glm::vec3 { 0.f, -1.f, 0.f });

        return glm::toMat4(yawRotation) * glm::toMat4(pitchRotation);
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
    float pitch_ = 0.0f;
    float yaw_ = 0.0f;
    float padding[9];

};

}

