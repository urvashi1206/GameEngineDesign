#pragma once

// lib
#include <ecs/Components.hpp>
#include <ecs/ECSCoordinator.hpp>

#include "GameObject.hpp"
#include "vulkan/vulkan.h"

namespace Minimal
{
#define MAX_LIGHTS 10

    struct PointLight
    {
        glm::vec4 position{}; // ignore w
        glm::vec4 color{}; // w is intensity
    };

    struct GlobalUBO
    {
        glm::mat4 projection{1.0f};
        glm::mat4 view{1.0f};
        glm::mat4 inverseView{1.0f};
        glm::vec4 ambientColor{0.1f, 0.1f, 0.1f, 0.2f}; // w is intensity
        PointLight pointLights[MAX_LIGHTS];
        int numLights;
    };

    struct FrameInfo
    {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        CameraComponent& camera;
        VkDescriptorSet globalDescriptorSet;
        // GameObject::Map &gameObjects;
        ECSCoordinator& ecs;
    };
}
