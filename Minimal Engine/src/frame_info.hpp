#pragma once

#include "camera.hpp"

// lib
#include <vulkan/vulkan.h>

#include "game_object.hpp"

namespace minimal {
#define MAX_LIGHTS 10

    struct point_light {
        glm::vec4 position{}; // ignore w
        glm::vec4 color{}; // w is intensity
    };

    struct global_ubo {
        glm::mat4 projection{1.0f};
        glm::mat4 view{1.0f};
        glm::vec4 ambient_color{0.1f, 0.1f, 0.1f, 0.2f}; // w is intensity
        point_light point_lights[MAX_LIGHTS];
        int num_lights;
    };

    struct frame_info {
        int frame_index;
        float frame_time;
        VkCommandBuffer command_buffer;
        camera &camera;
        VkDescriptorSet global_descriptor_set;
        game_object::map &game_objects;
    };
}
