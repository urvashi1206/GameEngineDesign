#pragma once

#include <memory>
#include "EntityManager.hpp"
#include "ComponentManager.hpp"

class ECSCoordinator {
public:
    ECSCoordinator() {
        m_EntityManager = std::make_unique<EntityManager>();
        m_ComponentManager = std::make_unique<ComponentManager>();
    }

    Entity CreateEntity() {
        return m_EntityManager->CreateEntity();
    }

    void DestroyEntity(Entity entity) {
        m_EntityManager->DestroyEntity(entity);
        m_ComponentManager->DestroyEntity(entity);
    }

    template <typename T>
    void RegisterComponent() {
        m_ComponentManager->RegisterComponent<T>();
    }

    template <typename T>
    void AddComponent(Entity entity, const T& component) {
        m_ComponentManager->AddComponent<T>(entity, component);
    }

    template <typename T>
    void RemoveComponent(Entity entity) {
        m_ComponentManager->RemoveComponent<T>(entity);
    }

    template <typename T>
    T& GetComponent(Entity entity) {
        return m_ComponentManager->GetComponent<T>(entity);
    }

    template <typename T>
    bool HasComponent(Entity entity) {
        return m_ComponentManager->HasComponent<T>(entity);
    }

private:
    std::unique_ptr<EntityManager> m_EntityManager;
    std::unique_ptr<ComponentManager> m_ComponentManager;
};
