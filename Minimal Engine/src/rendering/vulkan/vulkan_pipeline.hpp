#pragma once

#include <string>
#include <vector>

#include "vulkan_device.hpp"

namespace minimal {
    struct vulkan_pipeline_config_info {
        vulkan_pipeline_config_info() = default;

        vulkan_pipeline_config_info(const vulkan_pipeline_config_info &) = delete;

        vulkan_pipeline_config_info &operator=(const vulkan_pipeline_config_info &) = delete;

        std::vector<VkVertexInputBindingDescription> binding_descriptions;
        std::vector<VkVertexInputAttributeDescription> attribute_descriptions;

        VkPipelineViewportStateCreateInfo viewport_info;
        VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
        VkPipelineRasterizationStateCreateInfo rasterization_info;
        VkPipelineMultisampleStateCreateInfo multisample_info;
        VkPipelineColorBlendAttachmentState color_blend_attachment;
        VkPipelineColorBlendStateCreateInfo color_blend_info;
        VkPipelineDepthStencilStateCreateInfo depth_stencil_info;
        std::vector<VkDynamicState> dynamic_state_enables;
        VkPipelineDynamicStateCreateInfo dynamic_state_info;
        VkPipelineLayout pipeline_layout = nullptr;
        VkRenderPass render_pass = nullptr;
        uint32_t subpass = 0;
    };

    class vulkan_pipeline {
    public:
        vulkan_pipeline(vulkan_device &device,
                        const std::string &vert_path,
                        const std::string &frag_path,
                        const vulkan_pipeline_config_info &config_info);

        ~vulkan_pipeline();

        vulkan_pipeline(const vulkan_pipeline &) = delete;

        vulkan_pipeline &operator=(const vulkan_pipeline &) = delete;

        void bind(VkCommandBuffer command_buffer);

        static void default_pipeline_config_info(vulkan_pipeline_config_info &config_info);

        static void enable_alpha_blending(vulkan_pipeline_config_info &config_info);

    private:
        static std::vector<char> read_file(const std::string &file_path);

        void create_graphics_pipeline(const std::string &vert_path,
                                      const std::string &frag_path,
                                      const vulkan_pipeline_config_info &config_info);

        void create_shader_module(const std::vector<char> &code, VkShaderModule *shader_module);

        vulkan_device &device_;
        VkPipeline graphics_pipeline_;
        VkShaderModule vert_shader_module_;
        VkShaderModule frag_shader_module_;
    };
}
