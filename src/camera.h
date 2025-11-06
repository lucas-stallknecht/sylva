#pragma once

#include "window.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace sylva
{
    class Camera
    {
      public:
        Camera();

        void process_input(Window & window, float dt);
        [[nodiscard]] glm::mat4 get_proj_view(float aspect_ratio) const;

      private:
        void update_vectors();

        glm::vec3 position_{0.0f, 0.3f, 0.0f};
        glm::vec3 forward_{0.0f, 0.0f, -1.0f};
        glm::vec3 up_;
        glm::vec3 right_;
        float yaw_{-90.0f};
        float pitch_{0.0f};

        static constexpr float move_speed = 1.0f;
        static constexpr float mouse_sensitivity = 0.05f;
        static constexpr float fov = 70.0f;
        static constexpr float near_plane = 0.01f;
        static constexpr float far_plane = 10.0f;
    };
} // namespace sylva
