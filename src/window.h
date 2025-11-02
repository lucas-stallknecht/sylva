#pragma once

#include <daxa/daxa.hpp>
using namespace daxa::types;

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

namespace sylva
{
    class Window
    {
      public:
        explicit Window(char const * window_name, u32 width = 800, u32 height = 600);
        Window(Window const &) = default;
        Window(Window &&) = delete;
        Window & operator=(Window const &) = default;
        Window & operator=(Window &&) = delete;
        ~Window();

        void set_mouse_capture(bool should_capture) const;
        [[nodiscard]] bool should_close() const;
        void update() const;
        [[nodiscard]] GLFWwindow * get_glfw_window() const;
        [[nodiscard]] daxa::NativeWindowHandle get_native_window_handle() const;
        static daxa::NativeWindowPlatform get_native_platform();

        bool swapchain_out_of_date = false;
        bool minimized = false;

      private:
        GLFWwindow * window_;
        u32 width_, height_;
    };
}; // namespace sylva
