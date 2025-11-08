#include "camera.h"
#include <algorithm>

namespace sylva
{
    Camera::Camera() { update_vectors(); }

    void Camera::process_input(Window & window, float const dt)
    {
        glm::vec3 move_dir{0.0f};

        if (window.is_key_pressed(GLFW_KEY_W))
            move_dir += forward_;
        if (window.is_key_pressed(GLFW_KEY_S))
            move_dir -= forward_;
        if (window.is_key_pressed(GLFW_KEY_A))
            move_dir -= right_;
        if (window.is_key_pressed(GLFW_KEY_D))
            move_dir += right_;
        if (window.is_key_pressed(GLFW_KEY_SPACE))
            move_dir += up_;
        if (window.is_key_pressed(GLFW_KEY_LEFT_CONTROL))
            move_dir -= up_;

        if (glm::length(move_dir) > 0.0f)
        {
            position_ += glm::normalize(move_dir) * move_speed * dt;
        }

        if (window.is_mouse_captured())
        {
            auto const delta = window.get_mouse_delta() * mouse_sensitivity;

            yaw_ += delta.x;
            pitch_ += delta.y;

            pitch_ = std::clamp(pitch_, -89.0f, 89.0f);

            update_vectors();
        }
    }

    void Camera::update_vectors()
    {
        glm::vec3 dir;
        dir.x = cos(glm::radians(pitch_)) * cos(glm::radians(yaw_));
        dir.y = sin(glm::radians(pitch_));
        dir.z = cos(glm::radians(pitch_)) * sin(glm::radians(yaw_));

        forward_ = glm::normalize(dir);
        right_ = glm::normalize(glm::cross(forward_, glm::vec3{0.0f, 1.0f, 0.0f}));
        up_ = glm::normalize(glm::cross(right_, forward_));
    }

    glm::mat4 Camera::get_proj_view(float aspect_ratio) const
    {
        glm::mat4 proj = glm::perspective(glm::radians(fov), aspect_ratio, near_plane, far_plane);
        proj[1][1] *= -1.0f; // Flip Y for Vulkan clip space

        glm::mat4 view = glm::lookAt(position_, position_ + forward_, up_);
        return proj * view;
    }
} // namespace sylva
