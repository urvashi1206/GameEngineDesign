#include "Window.hpp"

#include <iostream>
#include <ostream>
#include <stdexcept>

namespace Minimal {
    Window::Window(std::string title, int width, int height) : m_title(title), m_width(width), m_height(height) {
        initWindow();
    }

    Window::~Window() {
        glfwDestroyWindow(m_glfWWindow);
        glfwTerminate();
    }

    void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
        if (glfwCreateWindowSurface(instance, m_glfWWindow, nullptr, surface) != VK_SUCCESS)
            throw std::runtime_error("failed to create window surface!");
    }

    void Window::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
        auto app_window = reinterpret_cast<class Window *>(glfwGetWindowUserPointer(window));
        app_window->m_frameBufferResized = true;
        app_window->m_width = width;
        app_window->m_height = height;
    }

    void Window::initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        m_glfWWindow = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(m_glfWWindow, this);
        glfwSetFramebufferSizeCallback(m_glfWWindow, framebufferResizeCallback);
    }
}
