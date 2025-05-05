#include "Engine.hpp"

#include "KeyboardMovementController.hpp"
#include "ecs/Components.hpp"
#include "systems/PhysicsSystem.hpp"
#include "systems/PointLightMover.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <chrono>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

namespace Minimal {
    Engine::Engine() {
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
        PhysicsSystem physicsSystem{m_ecs};
        PointLightMover pointLightMover{m_ecs};

        m_scheduler.Startup();

        physicsSystem.initialize();
        m_renderer.initialize();

        auto cameraEntity = m_ecs.createEntity();
        m_ecs.addComponent<CameraComponent>(cameraEntity, {true});

        auto &cameraTransform = m_ecs.getComponent<TransformComponent>(cameraEntity);
        cameraTransform.position.y = -0.5f;
        cameraTransform.position.z = -2.5f;

        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        Scheduler::QueueTask([&]() {
            // Counter *mainCounter = Scheduler::CreateCounter();
            while (!m_window.shouldClose()) {
                glfwPollEvents();

                auto newTime = std::chrono::high_resolution_clock::now();
                float deltaTime = std::chrono::duration<float>(newTime - currentTime).count();
                currentTime = newTime;

                cameraController.moveInPlaneXZ(m_window.getGlfwWindow(), deltaTime, cameraTransform);

                // Scheduler::QueueTask([&]() {
                //     pointLightMover.update(deltaTime);
                // }, TaskPriority::HIGH, mainCounter);
                // Scheduler::QueueTask([&]() {
                //     physicsSystem.update(deltaTime);
                // }, TaskPriority::HIGH, mainCounter);
                // // todo: should we wait for all the tasks to finish before rendering?
                // Scheduler::QueueTask([&]() {
                //     m_renderer.update(deltaTime);
                // }, TaskPriority::HIGH, mainCounter);
                // Scheduler::WaitForCounter(mainCounter);

                pointLightMover.update(deltaTime);
                physicsSystem.update(deltaTime);
                m_renderer.update(deltaTime);
            }

            m_renderer.shutdown();
            Scheduler::Shutdown();
        });

        m_scheduler.Run();
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
        m_ecs.addComponent<ColliderComponent>(cube, {EColliderType::Box, glm::vec3(0, 0, 0), glm::vec3(2.5f, 0.5f, 2.5f)});
        m_ecs.addComponent<RigidbodyComponent>(cube, {false, 1000, 0, 0.5f, 0.3f, glm::vec3(0, 0.0f, 0), glm::vec3(0, 0.0f, 0), glm::vec3(0, 0.0f, 0)});
        auto &cubeTransform = m_ecs.getComponent<TransformComponent>(cube);
        cubeTransform.position = {0.0f, -2.0f, 0.0f};
        cubeTransform.scale = {5.0f, 1.5f, 5.0f};

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
            m_ecs.addComponent<MeshRendererComponent>(object, {mesh});
            m_ecs.addComponent<ColliderComponent>(object, {EColliderType::Box, glm::vec3(0, 0, 0), glm::vec3(0.5f, 0.5f, 0.5f)});
            m_ecs.addComponent<RigidbodyComponent>(object, {false, 1, 0, 0.5f, 0.3f, glm::vec3(0, -1.8f, 0), glm::vec3(0, 0.0f, 0), glm::vec3(0, 0.0f, 0)});

            auto &transform = m_ecs.getComponent<TransformComponent>(object);
            transform.position = {0.0f, 0.0f, 0.0f};
            transform.rotate(glm::quat(glm::vec3(0, 0, 1.0f)));
            transform.scale = {0.5f, 0.5f, 0.5f};
        }
    }
}
