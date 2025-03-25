#include "model.hpp"

#include "utils.hpp"

//libs
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <iostream>
#include <unordered_map>

namespace std
{
    template <>
    struct hash<minimal::model::vertex>
    {
        size_t operator()(const minimal::model::vertex& v) const
        {
            size_t seed = 0;
            minimal::hash_combine(seed, v.position, v.color, v.normal, v.uv);
            return seed;
        }
    };
}

namespace minimal
{
    std::vector<VkVertexInputBindingDescription> model::vertex::get_binding_descriptions()
    {
        std::vector<VkVertexInputBindingDescription> binding_descriptions(1);
        binding_descriptions[0].binding = 0;
        binding_descriptions[0].stride = sizeof(vertex);
        binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return binding_descriptions;
    }

    std::vector<VkVertexInputAttributeDescription> model::vertex::get_attribute_descriptions()
    {
        std::vector<VkVertexInputAttributeDescription> attribute_descriptions(2);
        attribute_descriptions[0].binding = 0;
        attribute_descriptions[0].location = 0;
        attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[0].offset = offsetof(vertex, position);

        attribute_descriptions[1].binding = 0;
        attribute_descriptions[1].location = 1;
        attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[1].offset = offsetof(vertex, color);


        return attribute_descriptions;
    }

    void model::builder::load_model(const std::string& file_path)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        // auto i_stream = std::ifstream{file_path};
        // if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, &i_stream))
        //     throw std::runtime_error(warn + err);

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, file_path.c_str()))
            throw std::runtime_error(warn + err);

        vertices.clear();
        indices.clear();

        std::unordered_map<vertex, uint32_t> unique_vertices{};

        for (const auto& shape : shapes)
            for (const auto& index : shape.mesh.indices)
            {
                vertex vertex{};

                if (index.vertex_index >= 0)
                {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                    };

                    auto color_index = 3 * index.vertex_index + 2;
                    if (color_index < attrib.colors.size())
                        vertex.color = {
                            attrib.colors[color_index - 2],
                            attrib.colors[color_index - 1],
                            attrib.colors[color_index - 0]
                        };
                    else
                        vertex.color = {1.0f, 1.0f, 1.0f};
                }

                if (index.normal_index >= 0)
                {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                    };
                }

                if (index.texcoord_index >= 0)
                {
                    vertex.uv = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1]
                    };
                }

                if (unique_vertices.count(vertex) == 0)
                {
                    unique_vertices[vertex] = static_cast<uint32_t>(unique_vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(unique_vertices[vertex]);
            }
    }

    model::model(device& device, const builder& builder) : device_{device}
    {
        create_vertex_buffer(builder.vertices);
        create_index_buffer(builder.indices);
    }

    model::~model()
    {
        vkDestroyBuffer(device_.get_device(), vertex_buffer_, nullptr);
        vkFreeMemory(device_.get_device(), vertex_buffer_memory_, nullptr);

        if (has_index_buffer_)
        {
            vkDestroyBuffer(device_.get_device(), index_buffer_, nullptr);
            vkFreeMemory(device_.get_device(), index_buffer_memory_, nullptr);
        }
    }

    std::unique_ptr<model> model::create_model_from_file(device& device, const std::string& file_path)
    {
        builder builder{};
        builder.load_model(file_path);

        std::cout << "Vectex count: " << builder.vertices.size() << '\n';

        return std::make_unique<model>(device, builder);
    }

    void model::bind(VkCommandBuffer command_buffer)
    {
        VkBuffer buffers[] = {vertex_buffer_};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);

        if (has_index_buffer_)
            vkCmdBindIndexBuffer(command_buffer, index_buffer_, 0, VK_INDEX_TYPE_UINT32);
    }

    void model::draw(VkCommandBuffer command_buffer)
    {
        if (has_index_buffer_)
            vkCmdDrawIndexed(command_buffer, index_count_, 1, 0, 0, 0);
        else
            vkCmdDraw(command_buffer, vertex_count_, 1, 0, 0);
    }

    void model::create_vertex_buffer(const std::vector<vertex>& vertices)
    {
        vertex_count_ = vertices.size();
        assert(vertex_count_ >= 3 && "Vertex count must be at least 3");

        VkDeviceSize buffer_size = sizeof(vertices[0]) * vertex_count_;

        VkBuffer staging_buffer;
        VkDeviceMemory staging_buffer_memory;
        device_.createBuffer(buffer_size,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             staging_buffer,
                             staging_buffer_memory);

        void* data;
        vkMapMemory(device_.get_device(), staging_buffer_memory, 0, buffer_size, 0, &data);
        memcpy(data, vertices.data(), static_cast<size_t>(buffer_size));
        vkUnmapMemory(device_.get_device(), staging_buffer_memory);

        device_.createBuffer(buffer_size,
                             VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                             vertex_buffer_,
                             vertex_buffer_memory_);

        device_.copyBuffer(staging_buffer, vertex_buffer_, buffer_size);

        vkDestroyBuffer(device_.get_device(), staging_buffer, nullptr);
        vkFreeMemory(device_.get_device(), staging_buffer_memory, nullptr);
    }

    void model::create_index_buffer(const std::vector<uint32_t>& indices)
    {
        index_count_ = indices.size();
        has_index_buffer_ = index_count_ > 0;

        if (!has_index_buffer_) return;

        VkDeviceSize buffer_size = sizeof(indices[0]) * index_count_;


        VkBuffer staging_buffer;
        VkDeviceMemory staging_buffer_memory;
        device_.createBuffer(buffer_size,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             staging_buffer,
                             staging_buffer_memory);

        void* data;
        vkMapMemory(device_.get_device(), staging_buffer_memory, 0, buffer_size, 0, &data);
        memcpy(data, indices.data(), static_cast<size_t>(buffer_size));
        vkUnmapMemory(device_.get_device(), staging_buffer_memory);

        device_.createBuffer(buffer_size,
                             VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                             index_buffer_,
                             index_buffer_memory_);

        device_.copyBuffer(staging_buffer, index_buffer_, buffer_size);

        vkDestroyBuffer(device_.get_device(), staging_buffer, nullptr);
        vkFreeMemory(device_.get_device(), staging_buffer_memory, nullptr);
    }
}
