#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace Minimal {
    class Window {
    public:
        Window(std::string title, int width, int height);

        ~Window();

        Window(const Window &) = delete;

        Window &operator=(const Window &) = delete;

        bool shouldClose() { return glfwWindowShouldClose(m_glfWWindow); }
        VkExtent2D getExtent() { return {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height)}; }
        bool wasWindowResized() { return m_frameBufferResized; }
        void resetWindowResizedFlag() { m_frameBufferResized = false; }

        GLFWwindow *getGlfwWindow() const { return m_glfWWindow; }

        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

    private:
        std::string m_title;
        int m_width;
        int m_height;
        bool m_frameBufferResized = false;

        GLFWwindow *m_glfWWindow;

        static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

        void initWindow();
    };
}
