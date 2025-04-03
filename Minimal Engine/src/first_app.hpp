#pragma once

#include <memory>

#include "descriptors.hpp"
#include "rendering/vulkan/vulkan_device.hpp"
#include "game_object.hpp"
#include "window.hpp"
#include "rendering/vulkan/vulkan_renderer.hpp"

namespace minimal
{
    class first_app
    {
    public:
        static constexpr int width = 800;
        static constexpr int height = 600;

        first_app();
        ~first_app();

        first_app(const first_app &) = delete;
        first_app &operator=(const first_app &) = delete;

        void run();

    private:
        void load_game_objects();

        window window_{"Hello Vulkan!", width, height};
        vulkan_device device_{window_};
        vulkan_renderer renderer_{window_, device_};

        // note: order of declarations matters
        std::unique_ptr<descriptor_pool> global_pool_{};
        game_object::map game_objects_;
    };
}
