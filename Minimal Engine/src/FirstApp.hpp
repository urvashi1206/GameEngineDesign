#pragma once

#include <memory>

#include "Descriptors.hpp"
#include "rendering/vulkan/VulkanDevice.hpp"
#include "GameObject.hpp"
#include "Window.hpp"
#include "rendering/vulkan/VulkanRenderer.hpp"

namespace Minimal
{
    class FirstApp
    {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        FirstApp();
        ~FirstApp();

        FirstApp(const FirstApp &) = delete;
        FirstApp &operator=(const FirstApp &) = delete;

        void run();

    private:
        void loadGameObjects();

        Window m_window{"Hello Vulkan!", WIDTH, HEIGHT};
        VulkanDevice m_device{m_window};
        VulkanRenderer m_renderer{m_window, m_device};

        // note: order of declarations matters
        std::unique_ptr<DescriptorPool> m_globalPool{};
        GameObject::Map m_gameObjects;
    };
}
