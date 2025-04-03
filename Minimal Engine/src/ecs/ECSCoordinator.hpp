#pragma once

#include <memory>
#include "EntityManager.hpp"
#include "ComponentManager.hpp"

namespace Minimal {
    class ECSCoordinator {
    public:
        ECSCoordinator() {
            m_entityManager = std::make_unique<EntityManager>();
            m_componentManager = std::make_unique<ComponentManager>();
        }

        Entity createEntity() {
            return m_entityManager->createEntity();
        }

        void destroyEntity(Entity entity) {
            m_entityManager->DestroyEntity(entity);
            m_componentManager->DestroyEntity(entity);
        }

        template<typename T>
        void registerComponent() {
            m_componentManager->registerComponent<T>();
        }

        template<typename T>
        void addComponent(Entity entity, const T &component) {
            m_componentManager->AddComponent<T>(entity, component);
        }

        template<typename T>
        void removeComponent(Entity entity) {
            m_componentManager->RemoveComponent<T>(entity);
        }

        template<typename T>
        T &getComponent(Entity entity) {
            return m_componentManager->GetComponent<T>(entity);
        }

        template<typename T>
        bool hasComponent(Entity entity) {
            return m_componentManager->HasComponent<T>(entity);
        }

    private:
        std::unique_ptr<EntityManager> m_entityManager;
        std::unique_ptr<ComponentManager> m_componentManager;
    };
}
