#pragma once

#include <string>
#include <vector>

#include "VulkanDevice.hpp"

namespace Minimal {
    struct VulkanPipelineConfigInfo {
        VulkanPipelineConfigInfo() = default;

        VulkanPipelineConfigInfo(const VulkanPipelineConfigInfo &) = delete;

        VulkanPipelineConfigInfo &operator=(const VulkanPipelineConfigInfo &) = delete;

        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    class VulkanPipeline {
    public:
        VulkanPipeline(VulkanDevice &device,
                        const std::string &vertPath,
                        const std::string &fragPath,
                        const VulkanPipelineConfigInfo &configInfo);

        ~VulkanPipeline();

        VulkanPipeline(const VulkanPipeline &) = delete;

        VulkanPipeline &operator=(const VulkanPipeline &) = delete;

        void bind(VkCommandBuffer commandBuffer);

        static void defaultPipelineConfigInfo(VulkanPipelineConfigInfo &configInfo);

        static void enableAlphaBlending(VulkanPipelineConfigInfo &configInfo);

    private:
        static std::vector<char> readFile(const std::string &filePath);

        void createGraphicsPipeline(const std::string &vertPath,
                                      const std::string &fragPath,
                                      const VulkanPipelineConfigInfo &configInfo);

        void createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule);

        VulkanDevice &m_device;
        VkPipeline m_graphicsPipeline;
        VkShaderModule m_vertShaderModule;
        VkShaderModule m_fragShaderModule;
    };
}
