#include "PointLightSystem.hpp"

#include <array>
#include <iostream>
#include <map>
#include <stdexcept>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace Minimal {
    struct PointLightPushConstants {
        glm::vec4 position{};
        glm::vec4 color{};
        float radius;
    };

    PointLightSystem::PointLightSystem(ECSCoordinator& ecs,
                                       VulkanDevice &device,
                                       VkRenderPass renderPass,
                                       VkDescriptorSetLayout globalSetLayout) : System(ecs),
                                                                                m_device{device} {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    PointLightSystem::~PointLightSystem() {
        vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
    }

    void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PointLightPushConstants);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) !=
            VK_SUCCESS)
            throw std::runtime_error("failed to create pipeline layout!");
    }

    void PointLightSystem::createPipeline(VkRenderPass renderPass) {
        assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        VulkanPipelineConfigInfo pipelineConfig{};
        VulkanPipeline::defaultPipelineConfigInfo(pipelineConfig);
        VulkanPipeline::enableAlphaBlending(pipelineConfig);
        pipelineConfig.bindingDescriptions.clear();
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_pipelineLayout;
        m_pipeline = std::make_unique<VulkanPipeline>(m_device,
                                                      "shaders/point_light.vert.spv",
                                                      "shaders/point_light.frag.spv",
                                                      pipelineConfig);
    }

    void PointLightSystem::update(FrameInfo &frameInfo) {
        auto rotateLight = rotate(glm::mat4(1.0f),
                                  frameInfo.frameTime,
                                  {0.0f, -1.0f, 0.0f}
        );
        int lightIndex = 0;

        for (Entity e = 0; e < m_ecs.getEntityCount(); e++) {
            if (!m_ecs.hasComponent<PointLightComponent>(e))
                continue;

            auto &transform = m_ecs.getComponent<TransformComponent>(e);
            auto &pointLight = m_ecs.getComponent<PointLightComponent>(e);

            assert(lightIndex< MAX_LIGHTS && "Point lights exceed maximum specified");

            // update light position
            transform.position = glm::vec3(rotateLight * glm::vec4(transform.position, 1.0f));

            // copy light to ubo
            frameInfo.ubo.pointLights[lightIndex].position = glm::vec4(transform.position, 1.0f);
            frameInfo.ubo.pointLights[lightIndex].color = glm::vec4(pointLight.color, pointLight.lightIntensity);

            lightIndex++;
        }

        frameInfo.ubo.numLights = lightIndex;
    }

    void PointLightSystem::render(FrameInfo &frameInfo) {
        // sort lights
        std::map<float, Entity> sortedLights;


        for (Entity e = 0; e < m_ecs.getEntityCount(); e++) {
            if (!m_ecs.hasComponent<PointLightComponent>(e))
                continue;

            auto &transform = m_ecs.getComponent<TransformComponent>(e);

            // calculate distance
            auto offset = frameInfo.camera->getPosition() - transform.position;
            float distanceSquared = dot(offset, offset);
            sortedLights[distanceSquared] = e;
        }
        // for (auto& kv : frameInfo.gameObjects)
        // {
        //     auto& obj = kv.second;
        //     if (obj.pointLight == nullptr) continue;
        //
        //     // calculate distance
        //     auto offset = frameInfo.camera.getPosition() - obj.transform.position;
        //     float distanceSquared = glm::dot(offset, offset);
        //     sortedLights[distanceSquared] = obj.getId();
        // }

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

        // iterate through sorted lights in reverse order
        for (auto it = sortedLights.rbegin(); it != sortedLights.rend(); ++it) {
            // auto& obj = frameInfo.gameObjects.at(it->second);
            auto &transform = m_ecs.getComponent<TransformComponent>(it->second);
            auto &pointLight = m_ecs.getComponent<PointLightComponent>(it->second);

            PointLightPushConstants push{};

            push.position = glm::vec4(transform.position, 1.0f);
            push.color = glm::vec4(pointLight.color, pointLight.lightIntensity);
            push.radius = transform.scale.x;

            vkCmdPushConstants(
                frameInfo.commandBuffer,
                m_pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(PointLightPushConstants),
                &push
            );
            vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
        }
    }
}
