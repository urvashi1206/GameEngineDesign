#include "Engine.hpp"

#include "KeyboardMovementController.hpp"
#include "ecs/Components.hpp"
#include "rendering/vulkan/VulkanBuffer.hpp"
#include "systems/PointLightSystem.hpp"
#include "systems/SimpleRendererSystem.hpp"
#include "systems/PhysicsSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <chrono>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
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
        m_ecs.registerComponent<ColliderComponent>();
        m_ecs.registerComponent<RigidbodyComponent>();

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
            m_ecs,
            m_device,
            m_renderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout()
        };

        PointLightSystem pointLightSystem{
            m_ecs,
            m_device,
            m_renderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout()
        };

        PhysicsSystem physicsSystem(m_ecs);

        physicsSystem.initialize();

        CameraSystem cameraSystem{m_ecs};

        Entity cameraEntity = m_ecs.createEntity();
        m_ecs.addComponent<CameraComponent>(cameraEntity, {true});

        // cameraSystem.setViewDirection(glm::vec3(0.0f), glm::vec3(0.5f, 0.0f, 1.0f));
        cameraSystem.setViewTarget(cameraEntity, glm::vec3(-1.0f, -2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 2.5f));

        auto &cameraTransform = m_ecs.getComponent<TransformComponent>(cameraEntity);
        cameraTransform.position.y = -0.5f;
        cameraTransform.position.z = -2.5f;

        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!m_window.shouldClose()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(m_window.getGlfwWindow(), frameTime, cameraTransform);


            if (auto commandBuffer = m_renderer.beginFrame()) {
                int frameIndex = m_renderer.getFrameIndex();

                // update
                FrameInfo frameInfo{
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    m_renderer.getAspectRatio(),
                    {},
                    nullptr,
                    globalDescriptorSets[frameIndex]
                };

                cameraSystem.update(frameInfo);
                pointLightSystem.update(frameInfo);
                physicsSystem.update(frameInfo);

                uboBuffers[frameIndex]->writeToBuffer(&frameInfo.ubo);
                uboBuffers[frameIndex]->flush();

                // render
                m_renderer.beingSwapChainRenderPass(commandBuffer);

                simpleRendererSystem.render(frameInfo);
                pointLightSystem.render(frameInfo);

                m_renderer.endSwapChainRenderPass(commandBuffer);
                m_renderer.endFrame();
            }
        }

        vkDeviceWaitIdle(m_device.get_device());
    }

    void Engine::loadEntities() {
        // // Flat vase
        // std::shared_ptr mesh = Mesh::createModelFromFile(m_device, "models/flat_vase.obj");
        // auto flatVase = m_ecs.createEntity();
        // m_ecs.addComponent<MeshRendererComponent>(flatVase, {mesh});
        // auto &flatVaseTransform = m_ecs.getComponent<TransformComponent>(flatVase);
        // flatVaseTransform.position = {-0.5f, 0.5f, 0.0f};
        // flatVaseTransform.scale = {3.0f, 1.5f, 3.0f};

        // // Smooth vase
        // mesh = Mesh::createModelFromFile(m_device, "models/smooth_vase.obj");
        // auto smoothVase = m_ecs.createEntity();
        // m_ecs.addComponent<MeshRendererComponent>(smoothVase, {mesh});
        // auto &smoothVaseTransform = m_ecs.getComponent<TransformComponent>(smoothVase);
        // smoothVaseTransform.position = {0.5f, 0.5f, 0.0f};
        // smoothVaseTransform.scale = {3.0f, 1.5f, 3.0f};

        // // Floor
        // mesh = Mesh::createModelFromFile(m_device, "models/quad.obj");
        // auto floor = m_ecs.createEntity();
        // m_ecs.addComponent<MeshRendererComponent>(floor, {mesh});
        // auto &floorTransform = m_ecs.getComponent<TransformComponent>(floor);
        // floorTransform.position = {0.0f, 0.5f, 0.0f};
        // floorTransform.scale = {3.0f, 1.0f, 3.0f};


        // 24 unique vertices to allow distinct normal and UV per face.
        // Each face gets its own color, normal, and UV layout.
        std::vector<Mesh::Vertex> vertices = {
            // ----------------
            //     FRONT FACE
            // ----------------
            // Normal:  (0, 0, +1)
            // Color:   (1, 0, 0) (red face, just for example)
            // UVs:     simple 0→1 across X and Y
            {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // [0]
            {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // [1]
            {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // [2]
            {{-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // [3]
        
            // ----------------
            //     BACK FACE
            // ----------------
            // Normal:  (0, 0, -1)
            // Color:   (0, 1, 0) (green face)
            // Note we flip UVs as desired—this is one example
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}}, // [4]
            {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}}, // [5]
            {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}}, // [6]
            {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}}, // [7]
        
            // ----------------
            //     LEFT FACE
            // ----------------
            // Normal:  (-1, 0, 0)
            // Color:   (0, 0, 1) (blue face)
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // [8]
            {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // [9]
            {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}, // [10]
            {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // [11]
        
            // ----------------
            //     RIGHT FACE
            // ----------------
            // Normal:  (+1, 0, 0)
            // Color:   (1, 1, 0) (yellow face)
            {{0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // [12]
            {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // [13]
            {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}, // [14]
            {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // [15]
        
            // ----------------
            //     TOP FACE
            // ----------------
            // Normal:  (0, +1, 0)
            // Color:   (1, 0, 1) (magenta face)
            {{-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // [16]
            {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, // [17]
            {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // [18]
            {{-0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // [19]
        
            // ----------------
            //    BOTTOM FACE
            // ----------------
            // Normal:  (0, -1, 0)
            // Color:   (0, 1, 1) (cyan face)
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}, // [20]
            {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}}, // [21]
            {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}}, // [22]
            {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}}, // [23]
        };
        
        // Each face is two triangles. Hence, 6 faces × 2 triangles/face = 12 triangles.
        // Each triangle has 3 indices, so 12 × 3 = 36 indices total.
        std::vector<uint32_t> indices = {
            // Front face  (vertices 0,1,2,3)
            0, 1, 2, 2, 3, 0,
            // Back face   (vertices 4,5,6,7)
            4, 5, 6, 6, 7, 4,
            // Left face   (vertices 8,9,10,11)
            8, 9, 10, 10, 11, 8,
            // Right face  (vertices 12,13,14,15)
            12, 13, 14, 14, 15, 12,
            // Top face    (vertices 16,17,18,19)
            16, 17, 18, 18, 19, 16,
            // Bottom face (vertices 20,21,22,23)
            20, 21, 22, 22, 23, 20
        };
        
        
        std::shared_ptr mesh = Mesh::createModelFromVerticesAndIndices(m_device, vertices, indices);
        auto cube = m_ecs.createEntity();
        m_ecs.addComponent<MeshRendererComponent>(cube, {mesh});
        m_ecs.addComponent<ColliderComponent>(cube, { EColliderType::Box, glm::vec3(0, 0, 0), glm::vec3(0.5f, 0.5f, 0.5f) });
        m_ecs.addComponent<RigidbodyComponent>(cube, { true, 1, 0, 0.3f, 0.5f, glm::vec3(0, 0.0f, 0), glm::vec3(0, 0.0f, 0), glm::vec3(0, 0.0f, 0) });
        auto &cubeTransform = m_ecs.getComponent<TransformComponent>(cube);
        cubeTransform.position = {-0.5f, 0.5f, 0.0f};
        cubeTransform.scale = {3.0f, 1.5f, 3.0f};

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

        /* Physics objects */
        {
            auto object = m_ecs.createEntity();
            m_ecs.addComponent<MeshRendererComponent>(object, { mesh });
            m_ecs.addComponent<ColliderComponent>(object, { EColliderType::Box, glm::vec3(0, 0, 0), glm::vec3(0.5f, 0.5f, 0.5f) });
            m_ecs.addComponent<RigidbodyComponent>(object, { false, 1, 0, 0.3f, 0.5f, glm::vec3(0, 1.0f, 0), glm::vec3(0, 1.0f, 0), glm::vec3(0, 1.0f, 0) });

            auto& transform = m_ecs.getComponent<TransformComponent>(object);
            transform.position = { 0.0f, -1.5f, 0.0f };
            transform.scale = { 0.5f, 0.5f, 0.5f };
        }
    }
}
