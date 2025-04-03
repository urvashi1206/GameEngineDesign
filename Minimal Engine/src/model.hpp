#pragma once

#include "rendering/vulkan/vulkan_buffer.hpp"
#include "rendering/vulkan/vulkan_device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <memory>
#include <glm/glm.hpp>

namespace minimal {
    class model {
    public:
        struct vertex {
            glm::vec3 position{};
            glm::vec3 color{};
            glm::vec3 normal{};
            glm::vec2 uv{};

            static std::vector<VkVertexInputBindingDescription> get_binding_descriptions();

            static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();

            bool operator==(const vertex &other) const {
                return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
            }
        };

        struct builder {
            std::vector<vertex> vertices;
            std::vector<uint32_t> indices;

            void load_model(const std::string &file_path);
        };

        model(vulkan_device &device, const builder &builder);

        ~model();

        model(const model &) = delete;

        model &operator=(const model &) = delete;

        static std::unique_ptr<model> create_model_from_file(vulkan_device &device, const std::string &file_path);

        void bind(VkCommandBuffer command_buffer);

        void draw(VkCommandBuffer command_buffer);

    private:
        void create_vertex_buffer(const std::vector<vertex> &vertices);

        void create_index_buffer(const std::vector<uint32_t> &indices);

        vulkan_device &device_;

        std::unique_ptr<vulkan_buffer> vertex_buffer_;
        uint32_t vertex_count_;

        bool has_index_buffer_ = false;
        std::unique_ptr<vulkan_buffer> index_buffer_;
        uint32_t index_count_;
    };
}
