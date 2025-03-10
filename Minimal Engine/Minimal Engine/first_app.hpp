#pragma once

#include <memory>

#include "device.hpp"
#include "game_object.hpp"
#include "window.hpp"
#include "renderer.hpp"

namespace minimal
{
    class first_app
    {
    public:
        static constexpr int width = 800;
        static constexpr int height = 600;

        first_app();
        ~first_app();

        first_app(const first_app&) = delete;
        first_app& operator=(const first_app&) = delete;

        void run();

    private:
        void load_game_objects();

        window window_{"Hello Vulkan!", width, height};
        device device_{window_};
        renderer renderer_{window_, device_};
        
        std::vector<game_object> game_objects_;
    };
}
