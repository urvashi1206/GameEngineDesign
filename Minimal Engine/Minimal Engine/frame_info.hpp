#pragma once

#include "camera.hpp"

// lib
#include <vulkan/vulkan.h>

#include "game_object.hpp"

namespace minimal
{
    struct frame_info
    {
        int frame_index;
        float frame_time;
        VkCommandBuffer command_buffer;
        camera& camera;
        VkDescriptorSet global_descriptor_set;
        game_object::map& game_objects;
    };
}
