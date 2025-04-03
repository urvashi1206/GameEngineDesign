#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace minimal {
    class window {
    public:
        window(std::string title, int width, int height);

        ~window();

        window(const window &) = delete;

        window &operator=(const window &) = delete;

        bool should_close() { return glfwWindowShouldClose(glfw_window_); }
        VkExtent2D get_extent() { return {static_cast<uint32_t>(width_), static_cast<uint32_t>(height_)}; }
        bool was_window_resized() { return frame_buffer_resized_; }
        void reset_window_resized_flag() { frame_buffer_resized_ = false; }

        GLFWwindow *get_glfw_window() const { return glfw_window_; }

        void create_window_surface(VkInstance instance, VkSurfaceKHR *surface);

    private:
        std::string title_;
        int width_;
        int height_;
        bool frame_buffer_resized_ = false;

        GLFWwindow *glfw_window_;

        static void framebuffer_resize_callback(GLFWwindow *window, int width, int height);

        void init_window();
    };
}
