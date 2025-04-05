#include "SimpleRendererSystem.hpp"

#include <array>
#include <iostream>
#include <stdexcept>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace Minimal
{
    struct SimplePushConstantData
    {
        glm::mat4 modelMatrix{1.0f};
        glm::mat4 normalMatrix{1.0f};
    };

    SimpleRendererSystem::SimpleRendererSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : m_device{device}
    {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    SimpleRendererSystem::~SimpleRendererSystem()
    {
        vkDestroyPipelineLayout(m_device.get_device(), m_pipelineLayout, nullptr);
    }

    void SimpleRendererSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        std::vector<VkDescriptorSetLayout> descriptor_set_layouts{globalSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptor_set_layouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(m_device.get_device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("failed to create pipeline layout!");
    }

    void SimpleRendererSystem::createPipeline(VkRenderPass renderPass)
    {
        assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        VulkanPipelineConfigInfo pipelineConfig{};
        VulkanPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_pipelineLayout;
        m_pipeline = std::make_unique<VulkanPipeline>(m_device,
                                                      "shaders/simple_shader.vert.spv",
                                                      "shaders/simple_shader.frag.spv",
                                                      pipelineConfig);
    }

    void SimpleRendererSystem::renderGameObjects(FrameInfo& frameInfo)
    {
        m_pipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipelineLayout,
            0,
            1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr
        );

        for (Entity e = 0; e < MAX_ENTITIES; ++e)
        {
            if (frameInfo.ecs.hasComponent<MeshRendererComponent>(e))
            {
                auto& transform = frameInfo.ecs.getComponent<TransformComponent>(e);
                auto model = frameInfo.ecs.getComponent<MeshRendererComponent>(e).model;

                SimplePushConstantData push{};

                push.modelMatrix = transform.mat4();
                push.normalMatrix = transform.normalMatrix();

                vkCmdPushConstants(frameInfo.commandBuffer,
                                   m_pipelineLayout,
                                   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                   0,
                                   sizeof(push),
                                   &push);
                model->bind(frameInfo.commandBuffer);
                model->draw(frameInfo.commandBuffer);
            }
        }


        // for (auto& kv : frameInfo.gameObjects)
        // {
        //     auto& obj = kv.second;
        //     if (obj.model == nullptr) continue;
        //
        //     SimplePushConstantData push{};
        //
        //     push.modelMatrix = obj.transform.mat4();
        //     push.normalMatrix = obj.transform.normalMatrix();
        //
        //     vkCmdPushConstants(frameInfo.commandBuffer,
        //                        m_pipelineLayout,
        //                        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        //                        0,
        //                        sizeof(push),
        //                        &push);
        //     obj.model->bind(frameInfo.commandBuffer);
        //     obj.model->draw(frameInfo.commandBuffer);
        // }
    }
}
