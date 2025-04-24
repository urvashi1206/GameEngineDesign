#pragma once

#include <memory>
#include "EntityManager.hpp"
#include "ComponentManager.hpp"
#include "Components.hpp"
#include "scheduler/SpinLock.h"

namespace Minimal {
    class ECSCoordinator {
    public:
        ECSCoordinator() {
            m_entityManager = std::make_unique<EntityManager>();
            m_componentManager = std::make_unique<ComponentManager>();
        }

        Entity createEntity() {
            m_createEntityLock.Acquire();
            Entity entity = m_entityManager->createEntity();
            addComponent<TransformComponent>(entity, {});

            m_createEntityLock.Release();
            return entity;
        }

        void destroyEntity(Entity entity) {
            m_destroyEntityLock.Acquire();
            m_entityManager->destroyEntity(entity);
            m_componentManager->destroyEntity(entity);
            m_destroyEntityLock.Release();
        }

        int getEntityCount() const {
            return m_entityManager->getEntityCount();
        }

        template<typename T>
        void registerComponent() {
            m_registerComponentLock.Acquire();
            m_componentManager->registerComponent<T>();
            m_registerComponentLock.Release();
        }

        template<typename T>
        void addComponent(Entity entity, const T &component) {
            m_addComponentLock.Acquire();
            m_componentManager->addComponent<T>(entity, component);
            m_addComponentLock.Release();
        }

        template<typename T>
        void removeComponent(Entity entity) {
            m_removeComponentLock.Acquire();
            m_componentManager->removeComponent<T>(entity);
            m_removeComponentLock.Release();
        }

        template<typename T>
        T &getComponent(Entity entity) {
            m_getComponentLock.Acquire();
            T& component = m_componentManager->getComponent<T>(entity);
            m_getComponentLock.Release();
            return component;
        }

        template<typename T>
        bool hasComponent(Entity entity) {
            m_hasComponentLock.Acquire();
            bool result = m_componentManager->hasComponent<T>(entity);
            m_hasComponentLock.Release();
            return result;
        }


    private:
        std::unique_ptr<EntityManager> m_entityManager;
        std::unique_ptr<ComponentManager> m_componentManager;
        SpinLock m_createEntityLock;
        SpinLock m_destroyEntityLock;
        SpinLock m_getComponentLock;
        SpinLock m_hasComponentLock;
        SpinLock m_removeComponentLock;
        SpinLock m_addComponentLock;
        SpinLock m_registerComponentLock;
    };
}
