#pragma once

#include "rendering/vulkan/VulkanBuffer.hpp"
#include "rendering/vulkan/VulkanDevice.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <memory>
#include <glm/glm.hpp>

namespace Minimal {
    class Mesh {
    public:
        struct Vertex {
            glm::vec3 position{};
            glm::vec3 color{};
            glm::vec3 normal{};
            glm::vec2 uv{};

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();

            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

            bool operator==(const Vertex &other) const {
                return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
            }
        };

        struct Builder {
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;

            void loadModel(const std::string &filePath);
        };

        Mesh(VulkanDevice &device, const Builder &builder);

        ~Mesh();

        Mesh(const Mesh &) = delete;

        Mesh &operator=(const Mesh &) = delete;

        static std::unique_ptr<Mesh> createModelFromFile(VulkanDevice &device, const std::string &filePath);

        void bind(VkCommandBuffer commandBuffer);

        void draw(VkCommandBuffer commandBuffer);

    private:
        void createVertexBuffer(const std::vector<Vertex> &vertices);

        void createIndexBuffer(const std::vector<uint32_t> &indices);

        VulkanDevice &m_device;

        std::unique_ptr<VulkanBuffer> m_vertexBuffer;
        uint32_t m_vertexCount;

        bool m_hasIndexBuffer = false;
        std::unique_ptr<VulkanBuffer> m_indexBuffer;
        uint32_t m_indexCount;
    };
}
