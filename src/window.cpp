#include "window.h"

#include <GLFW/glfw3.h>

namespace sylva
{
    namespace
    {
        void glfw_window_size_callback(GLFWwindow * window, int width, int height)
        {
            auto * win = static_cast<Window *>(glfwGetWindowUserPointer(window));
            win->swapchain_out_of_date = true;
            win->minimized = (width == 0 || height == 0);
        }

        void glfw_key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
        {
            auto * win = static_cast<Window *>(glfwGetWindowUserPointer(window));
            win->on_key(key, action);
        }

        void glfw_cursor_pos_callback(GLFWwindow * window, double xpos, double ypos)
        {
            auto * win = static_cast<Window *>(glfwGetWindowUserPointer(window));
            win->on_mouse_move(static_cast<float>(xpos), static_cast<float>(ypos));
        }

        void glfw_mouse_button_callback(GLFWwindow * window, int button, int action, int mods)
        {
            auto * win = static_cast<Window *>(glfwGetWindowUserPointer(window));
            win->on_mouse_button(button, action);
        }
    } // namespace

    Window::Window(char const * window_name, u32 width, u32 height) : width_{width}, height_{height}
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window_ = glfwCreateWindow(static_cast<i32>(width_), static_cast<i32>(height_), window_name,
                                   nullptr, nullptr);

        glfwSetWindowUserPointer(window_, this);

        glfwSetWindowSizeCallback(window_, glfw_window_size_callback);
        glfwSetKeyCallback(window_, glfw_key_callback);
        glfwSetCursorPosCallback(window_, glfw_cursor_pos_callback);
        glfwSetMouseButtonCallback(window_, glfw_mouse_button_callback);
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

    void Window::on_key(int key, int action)
    {
        if (key >= 0 && key < static_cast<int>(keys_are_pressed_.size()))
        {
            keys_are_pressed_[key] = (action == GLFW_PRESS || action == GLFW_REPEAT);
        }
    }

    void Window::on_mouse_move(float xpos, float ypos)
    {
        if (!mouse_captured_)
        {
            return;
        }

        if (first_mouse_move_)
        {
            last_mouse_position_ = {xpos, ypos};
            first_mouse_move_ = false;
        }

        mouse_delta_ = {xpos - last_mouse_position_.x, last_mouse_position_.y - ypos};
        last_mouse_position_ = {xpos, ypos};
    }

    void Window::on_mouse_button(int button, int action)
    {
        if (button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            if (action == GLFW_PRESS)
            {
                mouse_captured_ = true;
                first_mouse_move_ = true;
                glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            else if (action == GLFW_RELEASE)
            {
                mouse_captured_ = false;
                glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }
    }

    bool Window::is_key_pressed(int key) const
    {
        if (key < 0 || key >= static_cast<int>(keys_are_pressed_.size()))
        {
            return false;
        }
        return keys_are_pressed_[key];
    }

    glm::vec2 Window::get_mouse_position() const
    {
        double x, y;
        glfwGetCursorPos(window_, &x, &y);
        return {static_cast<float>(x), static_cast<float>(y)};
    }

    glm::vec2 Window::get_mouse_delta()
    {
        glm::vec2 delta = mouse_delta_;
        mouse_delta_ = {0.0f, 0.0f};
        return delta;
    }

    bool Window::is_mouse_captured() const { return mouse_captured_; }

} // namespace sylva
