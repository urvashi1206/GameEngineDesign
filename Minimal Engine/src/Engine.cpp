#include "Engine.hpp"

#include "KeyboardMovementController.hpp"
#include "ecs/Components.hpp"
#include "rendering/vulkan/VulkanBuffer.hpp"
#include "systems/PointLightSystem.hpp"
#include "systems/SimpleRendererSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <algorithm>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "systems/CameraSystem.hpp"


namespace Minimal {
    Engine::Engine() {
        m_globalPool = VulkanDescriptorPool::Builder(m_device)
                .setMaxSets(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VulkanSwapChain::MAX_FRAMES_IN_FLIGHT)
                .build();

        m_ecs.registerComponent<TransformComponent>();
        m_ecs.registerComponent<CameraComponent>();
        m_ecs.registerComponent<MeshRendererComponent>();
        m_ecs.registerComponent<PointLightComponent>();

        loadEntities();
    }

    Engine::~Engine() = default;

    void Engine::run() {
        std::vector<std::unique_ptr<VulkanBuffer> > uboBuffers(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<VulkanBuffer>(
                m_device,
                sizeof(GlobalUBO),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            );
            uboBuffers[i]->map();
        }

        auto globalSetLayout = VulkanDescriptorSetLayout::Builder(m_device)
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < VulkanSwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            VulkanDescriptorWriter(*globalSetLayout, *m_globalPool)
                    .writeBuffer(0, &bufferInfo)
                    .build(globalDescriptorSets[i]);
        }

        SimpleRendererSystem simpleRendererSystem{
            m_device,
            m_renderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout()
        };

        PointLightSystem pointLightSystem{
            m_device,
            m_renderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout()
        };

        CameraSystem cameraSystem{m_ecs};

        Entity cameraEntity = m_ecs.createEntity();
        m_ecs.addComponent<CameraComponent>(cameraEntity, {true});

        // camera.set_view_direction(glm::vec3(0.0f), glm::vec3(0.5f, 0.0f, 1.0f));
        cameraSystem.setViewTarget(cameraEntity, glm::vec3(-1.0f, -2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 2.5f));

        auto &cameraTransform = m_ecs.getComponent<TransformComponent>(cameraEntity);
        cameraTransform.position.z = -2.5f;

        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!m_window.shouldClose()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(m_window.getGlfwWindow(), frameTime, cameraTransform);

            float aspect = m_renderer.getAspectRatio();
            cameraSystem.update(aspect);

            if (auto commandBuffer = m_renderer.beginFrame()) {
                int frameIndex = m_renderer.getFrameIndex();

                auto &mainCamera = cameraSystem.getMainCamera();

                FrameInfo frameInfo{
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    mainCamera,
                    globalDescriptorSets[frameIndex],
                    m_ecs
                };

                // update
                GlobalUBO ubo{};
                ubo.projection = mainCamera.projectionMatrix;
                ubo.view = mainCamera.viewMatrix;
                ubo.inverseView = mainCamera.inverseViewMatrix;
                pointLightSystem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // render
                m_renderer.beingSwapChainRenderPass(commandBuffer);

                simpleRendererSystem.renderGameObjects(frameInfo);
                pointLightSystem.render(frameInfo);

                m_renderer.endSwapChainRenderPass(commandBuffer);
                m_renderer.endFrame();
            }
        }

        vkDeviceWaitIdle(m_device.get_device());
    }

    void Engine::loadEntities() {
        // Flat vase
        std::shared_ptr model = Model::createModelFromFile(m_device, "models/flat_vase.obj");
        auto flatVase = m_ecs.createEntity();
        m_ecs.addComponent<MeshRendererComponent>(flatVase, {model});
        auto &flatVaseTransform = m_ecs.getComponent<TransformComponent>(flatVase);
        flatVaseTransform.position = {-0.5f, 0.5f, 0.0f};
        flatVaseTransform.scale = {3.0f, 1.5f, 3.0f};

        // Smooth vase
        model = Model::createModelFromFile(m_device, "models/smooth_vase.obj");
        auto smoothVase = m_ecs.createEntity();
        m_ecs.addComponent<MeshRendererComponent>(smoothVase, {model});
        auto &smoothVaseTransform = m_ecs.getComponent<TransformComponent>(smoothVase);
        smoothVaseTransform.position = {0.5f, 0.5f, 0.0f};
        smoothVaseTransform.scale = {3.0f, 1.5f, 3.0f};

        // Floor
        model = Model::createModelFromFile(m_device, "models/quad.obj");
        auto floor = m_ecs.createEntity();
        m_ecs.addComponent<MeshRendererComponent>(floor, {model});
        auto &floorTransform = m_ecs.getComponent<TransformComponent>(floor);
        floorTransform.position = {0.0f, 0.5f, 0.0f};
        floorTransform.scale = {3.0f, 1.0f, 3.0f};

        // Point lights
        std::vector<glm::vec3> lightColors{
            {1.0f, 0.1f, 0.1f},
            {0.1f, 0.1f, 1.0f},
            {0.1f, 1.0f, 0.1f},
            {1.0f, 1.0f, 0.1f},
            {0.1f, 1.0f, 1.0f},
            {1.0f, 1.0f, 1.0f}
        };

        for (int i = 0; i < lightColors.size(); i++) {
            auto pointLight = m_ecs.createEntity();
            m_ecs.addComponent<PointLightComponent>(pointLight, {0.2f, lightColors[i]});
            auto &pointLightTransform = m_ecs.getComponent<TransformComponent>(pointLight);
            pointLightTransform.scale.x = 0.1f;
            auto rotateLight = rotate(glm::mat4(1.0f),
                                      i * glm::two_pi<float>() / lightColors.size(),
                                      {0.0f, -1.0f, 0.0f});
            pointLightTransform.position = glm::vec3(rotateLight * glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f));
        }
    }
}
