#pragma once

#include "camera.hpp"

// lib
#include <vulkan/vulkan.h>

namespace minimal
{
    struct frame_info
    {
        int frame_index;
        float frame_time;
        VkCommandBuffer command_buffer;
        camera& camera;
        VkDescriptorSet global_descriptor_set;
    };
}
