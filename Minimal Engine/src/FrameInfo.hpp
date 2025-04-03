#pragma once

#include "Camera.hpp"

// lib
#include <vulkan/vulkan.h>

#include "GameObject.hpp"

namespace Minimal {
#define MAX_LIGHTS 10

    struct PointLight {
        glm::vec4 position{}; // ignore w
        glm::vec4 color{}; // w is intensity
    };

    struct GlobalUBO {
        glm::mat4 projection{1.0f};
        glm::mat4 view{1.0f};
        glm::mat4 inverseView{1.0f};
        glm::vec4 ambientColor{0.1f, 0.1f, 0.1f, 0.2f}; // w is intensity
        PointLight pointLights[MAX_LIGHTS];
        int numLights;
    };

    struct FrameInfo {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        Camera &camera;
        VkDescriptorSet globalDescriptorSet;
        GameObject::Map &gameObjects;
    };
}
