#include "window.hpp"

#include <iostream>
#include <ostream>
#include <stdexcept>

namespace minimal
{
    window::window(std::string title, int width, int height) : title_(title), width_(width), height_(height)
    {
        init_window();
    }

    window::~window()
    {
        glfwDestroyWindow(glfw_window_);
        glfwTerminate();
    }

    void window::create_window_surface(VkInstance instance, VkSurfaceKHR* surface)
    {
        if (glfwCreateWindowSurface(instance, glfw_window_, nullptr, surface) != VK_SUCCESS)
            throw std::runtime_error("failed to create window surface!");
    }

    void window::framebuffer_resize_callback(GLFWwindow* window, int width, int height)
    {
        auto app_window = reinterpret_cast<class window*>(glfwGetWindowUserPointer(window));
        app_window->frame_buffer_resized_ = true;
        app_window->width_ = width;
        app_window->height_ = height;
    }

    void window::init_window()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        glfw_window_ = glfwCreateWindow(width_, height_, title_.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(glfw_window_, this);
        glfwSetFramebufferSizeCallback(glfw_window_, framebuffer_resize_callback);
    }
}
