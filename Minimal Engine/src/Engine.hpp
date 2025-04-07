#pragma once

#include <memory>

#include "GameObject.hpp"
#include "Window.hpp"
#include "ecs/ECSCoordinator.hpp"
#include "rendering/vulkan/VulkanDescriptors.hpp"
#include "rendering/vulkan/VulkanDevice.hpp"
#include "rendering/vulkan/VulkanRenderer.hpp"

namespace Minimal {
    class Engine {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        /**
         * Constructor for the Engine class.
         * Initializes the Vulkan engine with a window and device.
         */
        Engine();

        ~Engine();

        Engine(const Engine &) = delete;

        Engine &operator=(const Engine &) = delete;

        void run();

    private:
        void loadEntities();

        ECSCoordinator m_ecs;
        Window m_window{"Hello Vulkan!", WIDTH, HEIGHT};
        VulkanDevice m_device{m_window};
        VulkanRenderer m_renderer{m_window, m_device};

        // note: order of declarations matters
        std::unique_ptr<VulkanDescriptorPool> m_globalPool{};
    };
}
