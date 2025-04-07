#pragma once
#include <memory>

#include "Model.hpp"

//libs
#include <unordered_map>
#include <glm/gtc/matrix_transform.hpp>

#include "ecs/Components.hpp"

namespace Minimal {

    class GameObject {
    public:
        using IdT = unsigned int;
        using Map = std::unordered_map<IdT, GameObject>;

        static GameObject create() {
            static IdT currentId = 0;
            return GameObject{currentId++};
        }

        static GameObject makePointLight(float intensity = 10.0f, float radius = 0.1f, glm::vec3 color = glm::vec3{1.0f});

        GameObject(const GameObject &) = delete;

        GameObject &operator=(const GameObject &) = delete;

        GameObject(GameObject &&) = default;

        GameObject &operator=(GameObject &&) = default;

        IdT getId() const { return m_id; }

        glm::vec3 color{};
        TransformComponent transform{};

        // Optional pointer components;
        std::shared_ptr<Model> model{};
        std::unique_ptr<PointLightComponent> pointLight = nullptr;

    private:
        GameObject(IdT objId) : m_id{objId} {}

        IdT m_id;
    };
}
