#pragma once
#include "device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace minimal
{
    class model
    {
    public:
        struct vertex
        {
            glm::vec3 position;
            glm::vec3 color;

            static std::vector<VkVertexInputBindingDescription> get_binding_descriptions();
            static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();
        };

        model(device& device, const std::vector<vertex>& vertices);
        ~model();

        model(const model&) = delete;
        model& operator=(const model&) = delete;

        void bind(VkCommandBuffer command_buffer);
        void draw(VkCommandBuffer command_buffer);

    private:
        void create_vertex_buffer(const std::vector<vertex>& vertices);

        device& device_;
        VkBuffer vertex_buffer_;
        VkDeviceMemory vertex_buffer_memory_;
        uint32_t vertex_count_;
    };
}
