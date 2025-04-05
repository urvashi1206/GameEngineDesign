#include "Model.hpp"

#include "Utils.hpp"

//libs
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <iostream>
#include <unordered_map>


namespace std {
    template<>
    struct hash<Minimal::Model::Vertex> {
        size_t operator()(const Minimal::Model::Vertex &v) const {
            size_t seed = 0;
            Minimal::hashCombine(seed, v.position, v.color, v.normal, v.uv);
            return seed;
        }
    };
}

namespace Minimal {
    std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
        attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
        attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
        attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

        return attributeDescriptions;
    }

    void Model::Builder::loadModel(const std::string &filePath) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        // auto i_stream = std::ifstream{file_path};
        // if (!LoadObj(&attrib, &shapes, &materials, &warn, &err, &i_stream))
        //     throw std::runtime_error(warn + err);

        if (!LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.c_str()))
            throw std::runtime_error(warn + err);

        vertices.clear();
        indices.clear();

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto &shape: shapes)
            for (const auto &index: shape.mesh.indices) {
                Vertex vertex{};

                if (index.vertex_index >= 0) {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                    };

                    vertex.color = {
                        attrib.colors[3 * index.vertex_index + 0],
                        attrib.colors[3 * index.vertex_index + 1],
                        attrib.colors[3 * index.vertex_index + 2]
                    };
                }

                if (index.normal_index >= 0) {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                    };
                }

                if (index.texcoord_index >= 0) {
                    vertex.uv = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1]
                    };
                }

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(uniqueVertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);
            }
    }

    Model::Model(VulkanDevice &device, const Builder &builder) : m_device{device} {
        createVertexBuffer(builder.vertices);
        createIndexBuffer(builder.indices);
    }

    Model::~Model() {}

    std::unique_ptr<Model> Model::createModelFromFile(VulkanDevice &device, const std::string &filePath) {
        Builder builder{};
        builder.loadModel(filePath);

        std::cout << "Vertex count: " << builder.vertices.size() << '\n';

        return std::make_unique<Model>(device, builder);
    }

    void Model::bind(VkCommandBuffer commandBuffer) {
        VkBuffer buffers[] = {m_vertexBuffer->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if (m_hasIndexBuffer)
            vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
    }

    void Model::draw(VkCommandBuffer commandBuffer) {
        if (m_hasIndexBuffer)
            vkCmdDrawIndexed(commandBuffer, m_indexCount, 1, 0, 0, 0);
        else
            vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0);
    }

    void Model::createVertexBuffer(const std::vector<Vertex> &vertices) {
        m_vertexCount = vertices.size();
        assert(m_vertexCount >= 3 && "Vertex count must be at least 3");

        VkDeviceSize bufferSize = sizeof(vertices[0]) * m_vertexCount;
        uint32_t vertexSize = sizeof(vertices[0]);

        VulkanBuffer stagingBuffer{
            m_device,
            vertexSize,
            m_vertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *) vertices.data());

        m_vertexBuffer = std::make_unique<VulkanBuffer>(
            m_device,
            vertexSize,
            m_vertexCount,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        m_device.copyBuffer(stagingBuffer.getBuffer(), m_vertexBuffer->getBuffer(), bufferSize);
    }

    void Model::createIndexBuffer(const std::vector<uint32_t> &indices) {
        m_indexCount = indices.size();
        m_hasIndexBuffer = m_indexCount > 0;

        if (!m_hasIndexBuffer) return;

        VkDeviceSize bufferSize = sizeof(indices[0]) * m_indexCount;
        uint32_t indexSize = sizeof(indices[0]);

        VulkanBuffer stagingBuffer{
            m_device,
            indexSize,
            m_indexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *) indices.data());

        m_indexBuffer = std::make_unique<VulkanBuffer>(
            m_device,
            indexSize,
            m_indexCount,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        m_device.copyBuffer(stagingBuffer.getBuffer(), m_indexBuffer->getBuffer(), bufferSize);
    }
}
