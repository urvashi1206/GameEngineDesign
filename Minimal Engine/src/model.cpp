#include "model.hpp"

#include "utils.hpp"

//libs
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <iostream>
#include <unordered_map>

#ifndef ENINGE_DIR
#define ENGINE_DIR "../"
#endif


namespace std {
    template<>
    struct hash<minimal::model::vertex> {
        size_t operator()(const minimal::model::vertex &v) const {
            size_t seed = 0;
            minimal::hash_combine(seed, v.position, v.color, v.normal, v.uv);
            return seed;
        }
    };
}

namespace minimal {
    std::vector<VkVertexInputBindingDescription> model::vertex::get_binding_descriptions() {
        std::vector<VkVertexInputBindingDescription> binding_descriptions(1);
        binding_descriptions[0].binding = 0;
        binding_descriptions[0].stride = sizeof(vertex);
        binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return binding_descriptions;
    }

    std::vector<VkVertexInputAttributeDescription> model::vertex::get_attribute_descriptions() {
        std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};

        attribute_descriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex, position)});
        attribute_descriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex, color)});
        attribute_descriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex, normal)});
        attribute_descriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex, uv)});

        return attribute_descriptions;
    }

    void model::builder::load_model(const std::string &file_path) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        // auto i_stream = std::ifstream{file_path};
        // if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, &i_stream))
        //     throw std::runtime_error(warn + err);

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, (ENGINE_DIR + file_path).c_str()))
            throw std::runtime_error(warn + err);

        vertices.clear();
        indices.clear();

        std::unordered_map<vertex, uint32_t> unique_vertices{};

        for (const auto &shape: shapes)
            for (const auto &index: shape.mesh.indices) {
                vertex vertex{};

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

                if (unique_vertices.count(vertex) == 0) {
                    unique_vertices[vertex] = static_cast<uint32_t>(unique_vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(unique_vertices[vertex]);
            }
    }

    model::model(device &device, const builder &builder) : device_{device} {
        create_vertex_buffer(builder.vertices);
        create_index_buffer(builder.indices);
    }

    model::~model() {
    }

    std::unique_ptr<model> model::create_model_from_file(device &device, const std::string &file_path) {
        builder builder{};
        builder.load_model(file_path);

        std::cout << "Vectex count: " << builder.vertices.size() << '\n';

        return std::make_unique<model>(device, builder);
    }

    void model::bind(VkCommandBuffer command_buffer) {
        VkBuffer buffers[] = {vertex_buffer_->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);

        if (has_index_buffer_)
            vkCmdBindIndexBuffer(command_buffer, index_buffer_->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
    }

    void model::draw(VkCommandBuffer command_buffer) {
        if (has_index_buffer_)
            vkCmdDrawIndexed(command_buffer, index_count_, 1, 0, 0, 0);
        else
            vkCmdDraw(command_buffer, vertex_count_, 1, 0, 0);
    }

    void model::create_vertex_buffer(const std::vector<vertex> &vertices) {
        vertex_count_ = vertices.size();
        assert(vertex_count_ >= 3 && "Vertex count must be at least 3");

        VkDeviceSize buffer_size = sizeof(vertices[0]) * vertex_count_;
        uint32_t vertex_size = sizeof(vertices[0]);

        buffer staging_buffer{
            device_,
            vertex_size,
            vertex_count_,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };

        staging_buffer.map();
        staging_buffer.writeToBuffer((void *) vertices.data());

        vertex_buffer_ = std::make_unique<buffer>(
            device_,
            vertex_size,
            vertex_count_,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        device_.copyBuffer(staging_buffer.getBuffer(), vertex_buffer_->getBuffer(), buffer_size);
    }

    void model::create_index_buffer(const std::vector<uint32_t> &indices) {
        index_count_ = indices.size();
        has_index_buffer_ = index_count_ > 0;

        if (!has_index_buffer_) return;

        VkDeviceSize buffer_size = sizeof(indices[0]) * index_count_;
        uint32_t index_size = sizeof(indices[0]);

        buffer staging_buffer{
            device_,
            index_size,
            index_count_,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        staging_buffer.map();
        staging_buffer.writeToBuffer((void *) indices.data());

        index_buffer_ = std::make_unique<buffer>(
            device_,
            index_size,
            index_count_,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        device_.copyBuffer(staging_buffer.getBuffer(), index_buffer_->getBuffer(), buffer_size);
    }
}
