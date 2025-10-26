#include "window.h"

#include <GLFW/glfw3.h>

namespace sylva
{
    Window::Window(char const * window_name, u32 width, u32 height) : width_{width}, height_{height}
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window_ = glfwCreateWindow(static_cast<i32>(width_), static_cast<i32>(height_), window_name,
                                   nullptr, nullptr);

        glfwSetWindowUserPointer(window_, this);
        // Set a callback to handle resizing events
        glfwSetWindowSizeCallback(window_,
                                  [](GLFWwindow * window, int size_x, int size_y)
                                  {
                                      auto * win =
                                          static_cast<Window *>(glfwGetWindowUserPointer(window));
                                      win->width_ = static_cast<u32>(size_x);
                                      win->height_ = static_cast<u32>(size_y);
                                      win->swapchain_out_of_date = true;
                                  });
    }

    Window::~Window()
    {
        glfwDestroyWindow(window_);
        glfwTerminate();
    }

    daxa::NativeWindowHandle Window::get_native_window_handle() const
    {
#if defined(_WIN32)
        return glfwGetWin32Window(window_);
#elif defined(__linux__)
        switch (get_native_platform())
        {
        case daxa::NativeWindowPlatform::WAYLAND_API:
            return reinterpret_cast<daxa::NativeWindowHandle>(glfwGetWaylandWindow(window_));
        case daxa::NativeWindowPlatform::XLIB_API:
        default: return reinterpret_cast<daxa::NativeWindowHandle>(glfwGetX11Window(window_));
        }
#endif
    }

    daxa::NativeWindowPlatform Window::get_native_platform()
    {
        switch (glfwGetPlatform())
        {
        case GLFW_PLATFORM_WIN32: return daxa::NativeWindowPlatform::WIN32_API;
        case GLFW_PLATFORM_X11: return daxa::NativeWindowPlatform::XLIB_API;
        case GLFW_PLATFORM_WAYLAND: return daxa::NativeWindowPlatform::WAYLAND_API;
        default: return daxa::NativeWindowPlatform::UNKNOWN;
        }
    }

    void Window::set_mouse_capture(bool should_capture) const
    {
        glfwSetCursorPos(window_, static_cast<double>(width_ / 2.),
                         static_cast<double>(height_ / 2.));
        glfwSetInputMode(window_, GLFW_CURSOR,
                         should_capture ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        glfwSetInputMode(window_, GLFW_RAW_MOUSE_MOTION, should_capture ? GLFW_TRUE : GLFW_FALSE);
    }

    bool Window::should_close() const { return glfwWindowShouldClose(window_); }

    void Window::update() const
    {
        glfwPollEvents();
        glfwSwapBuffers(window_);
    }

    GLFWwindow * Window::get_glfw_window() const { return window_; }

} // namespace sylva
