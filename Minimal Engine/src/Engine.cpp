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
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


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

        // loadGameObjects();
        loadEntities();
    }

    Engine::~Engine() = default;

    void Engine::run() {
        /*std::vector<std::unique_ptr<VulkanBuffer>> uboBuffers(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++)
        {
            uboBuffers[i] = std::make_unique<VulkanBuffer>(
                m_device,
                sizeof(GlobalUBO),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            );
            uboBuffers[i]->map();
        }

        auto globalSetLayout = DescriptorSetLayout::Builder(m_device)
                               .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                               .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < VulkanSwapChain::MAX_FRAMES_IN_FLIGHT; i++)
        {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            DescriptorWriter(*globalSetLayout, *m_globalPool)
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

        Camera camera{};
        // camera.set_view_direction(glm::vec3(0.0f), glm::vec3(0.5f, 0.0f, 1.0f));
        camera.setViewTarget(glm::vec3(-1.0f, -2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 2.5f));

        auto viewerObject = GameObject::create();
        viewerObject.transform.position.z = -2.5f;

        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!m_window.shouldClose())
        {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(m_window.getGlfwWindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.position, viewerObject.transform.rotation);

            float aspect = m_renderer.getAspectRatio();
            // camera.set_othrographic_projection(-aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f);
            camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 100.0f);

            if (auto commandBuffer = m_renderer.beginFrame())
            {
                int frameIndex = m_renderer.getFrameIndex();

                FrameInfo frameInfo{
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    camera,
                    globalDescriptorSets[frameIndex],
                    m_gameObjects
                };

                // update
                GlobalUBO ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
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

        vkDeviceWaitIdle(m_device.device());*/
    }

    void Engine::runECS() {
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

        Entity cameraEntity = m_ecs.createEntity();
        m_ecs.addComponent<CameraComponent>(cameraEntity, {});
        m_ecs.addComponent<TransformComponent>(cameraEntity, {});
        auto &camera = m_ecs.getComponent<CameraComponent>(cameraEntity);

        // camera.set_view_direction(glm::vec3(0.0f), glm::vec3(0.5f, 0.0f, 1.0f));
        camera.setViewTarget(glm::vec3(-1.0f, -2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 2.5f));

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
            camera.setViewYXZ(cameraTransform.position, cameraTransform.rotation);

            float aspect = m_renderer.getAspectRatio();
            // camera.set_othrographic_projection(-aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f);
            camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 100.0f);

            if (auto commandBuffer = m_renderer.beginFrame()) {
                int frameIndex = m_renderer.getFrameIndex();

                FrameInfo frameInfo{
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    camera,
                    globalDescriptorSets[frameIndex],
                    m_ecs
                };

                // update
                GlobalUBO ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                pointLightSystem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // auto test = ubo.inverseView;
                //
                // std::cout << "Test: \n";
                // for (uint8_t i = 0; i < 4; i++) {
                //     for (uint8_t j = 0; j < 4; j++) {
                //         std::cout << test[i][j] << " ";
                //     }
                //     std::cout << '\n';
                // }

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

    void Engine::loadGameObjects() {
        // Flat vase
        std::shared_ptr model = Model::createModelFromFile(m_device, "models/flat_vase.obj");
        auto flatVase = GameObject::create();
        flatVase.model = model;
        flatVase.transform.position = {-0.5f, 0.5f, 0.0f};
        flatVase.transform.scale = {3.0f, 1.5f, 3.0f};
        m_gameObjects.emplace(flatVase.getId(), std::move(flatVase));

        // Smooth vase
        model = Model::createModelFromFile(m_device, "models/smooth_vase.obj");
        auto smoothVase = GameObject::create();
        smoothVase.model = model;
        smoothVase.transform.position = {0.5f, 0.5f, 0.0f};
        smoothVase.transform.scale = {3.0f, 1.5f, 3.0f};
        m_gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

        // Floor
        model = Model::createModelFromFile(m_device, "models/quad.obj");
        auto floor = GameObject::create();
        floor.model = model;
        floor.transform.position = {0.0f, 0.5f, 0.0f};
        floor.transform.scale = {3.0f, 1.0f, 3.0f};
        m_gameObjects.emplace(floor.getId(), std::move(floor));

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
            auto pointLight = GameObject::makePointLight(0.2f);
            pointLight.color = lightColors[i];
            auto rotateLight = rotate(glm::mat4(1.0f),
                                      i * glm::two_pi<float>() / lightColors.size(),
                                      {0.0f, -1.0f, 0.0f});
            pointLight.transform.position = glm::vec3(rotateLight * glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f));
            m_gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }
    }

    void Engine::loadEntities() {
        // Flat vase
        std::shared_ptr model = Model::createModelFromFile(m_device, "models/flat_vase.obj");
        auto flatVase = m_ecs.createEntity();
        m_ecs.addComponent<MeshRendererComponent>(flatVase, {model});
        m_ecs.addComponent<TransformComponent>(flatVase, {});
        auto &flatVaseTransform = m_ecs.getComponent<TransformComponent>(flatVase);
        flatVaseTransform.position = {-0.5f, 0.5f, 0.0f};
        flatVaseTransform.scale = {3.0f, 1.5f, 3.0f};

        // Smooth vase
        model = Model::createModelFromFile(m_device, "models/smooth_vase.obj");
        auto smoothVase = m_ecs.createEntity();
        m_ecs.addComponent<MeshRendererComponent>(smoothVase, {model});
        m_ecs.addComponent<TransformComponent>(smoothVase, {});
        auto &smoothVaseTransform = m_ecs.getComponent<TransformComponent>(smoothVase);
        smoothVaseTransform.position = {0.5f, 0.5f, 0.0f};
        smoothVaseTransform.scale = {3.0f, 1.5f, 3.0f};

        // Floor
        model = Model::createModelFromFile(m_device, "models/quad.obj");
        auto floor = m_ecs.createEntity();
        m_ecs.addComponent<MeshRendererComponent>(floor, {model});
        m_ecs.addComponent<TransformComponent>(floor, {});
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
            m_ecs.addComponent<TransformComponent>(pointLight, {});
            auto &pointLightTransform = m_ecs.getComponent<TransformComponent>(pointLight);
            pointLightTransform.scale.x = 0.1f;
            auto rotateLight = rotate(glm::mat4(1.0f),
                                      i * glm::two_pi<float>() / lightColors.size(),
                                      {0.0f, -1.0f, 0.0f});
            pointLightTransform.position = glm::vec3(rotateLight * glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f));
        }
    }
}
