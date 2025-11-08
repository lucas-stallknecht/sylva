#pragma once

#include <daxa/daxa.hpp>
#include <cstdint>

#include <GLFW/glfw3.h>

#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_NATIVE_INCLUDE_NONE
using HWND = void *;
#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_WAYLAND
#endif

#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>

namespace sylva
{
    class Window
    {
      public:
        explicit Window(char const * window_name, std::uint32_t width = 800,
                        std::uint32_t height = 600);
        Window(Window const &) = default;
        Window(Window &&) = delete;
        Window & operator=(Window const &) = default;
        Window & operator=(Window &&) = delete;
        ~Window();

        void on_key(int key, int action);
        void on_mouse_move(float xpos, float ypos);
        void on_mouse_button(int button, int action);

        void set_mouse_capture(bool should_capture) const;
        [[nodiscard]] bool should_close() const;
        void update() const;
        [[nodiscard]] GLFWwindow * get_glfw_window() const;
        [[nodiscard]] daxa::NativeWindowHandle get_native_window_handle() const;
        static daxa::NativeWindowPlatform get_native_platform();

        [[nodiscard]] bool is_key_pressed(int key) const;
        [[nodiscard]] glm::vec2 get_mouse_position() const;
        [[nodiscard]] glm::vec2 get_mouse_delta();
        [[nodiscard]] bool is_mouse_captured() const;

        bool swapchain_out_of_date = false;
        bool minimized = false;

      private:
        GLFWwindow * window_;
        std::uint32_t width_, height_;
        std::array<bool, 512> keys_are_pressed_{};
        bool mouse_captured_ = false;
        bool first_mouse_move_ = true;
        glm::vec2 last_mouse_position_{0.0f};
        glm::vec2 mouse_delta_{0.0f};
    };
}; // namespace sylva
