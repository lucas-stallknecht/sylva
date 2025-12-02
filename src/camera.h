#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "window.h"
#include "defaults.h"

namespace sylva
{
    class Camera
    {
      public:
        Camera();

        void process_input(Window & window, float dt);
        [[nodiscard]] glm::mat4 get_proj(float aspect_ratio) const;
        [[nodiscard]] glm::mat4 get_view() const;
        [[nodiscard]] glm::vec3 get_position() const;

      private:
        void update_vectors();

        // Use centralized defaults for initial camera transform and orientation.
        glm::vec3 position_{defaults::camera_initial_position};
        glm::vec3 forward_{0.0f, 0.0f, -1.0f};
        glm::vec3 up_;
        glm::vec3 right_;
        float yaw_{defaults::camera_initial_yaw};
        float pitch_{defaults::camera_initial_pitch};

        static constexpr float move_speed = defaults::camera_move_speed;
        static constexpr float mouse_sensitivity = defaults::camera_mouse_sensitivity;
        static constexpr float fov = defaults::camera_fov;
        static constexpr float near_plane = defaults::camera_near_plane;
        static constexpr float far_plane = defaults::camera_far_plane;
    };
} // namespace sylva
