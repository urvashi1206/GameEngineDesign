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
            m_createentitylock.Acquire();
            Entity entity = m_entityManager->createEntity();
            addComponent<TransformComponent>(entity, {});

            m_createentitylock.Release();
            return entity;
        }

        void destroyEntity(Entity entity) {
            m_destroyentitylock.Acquire();
            m_entityManager->destroyEntity(entity);
            m_componentManager->destroyEntity(entity);
            m_destroyentitylock.Release();
        }

        int GetEntityCount() {
            return m_entityManager->GetEntityCount();
        }

        template<typename T>
        void registerComponent() {
            m_registerComponentlock.Acquire();
            m_componentManager->registerComponent<T>();
            m_registerComponentlock.Release();
        }

        template<typename T>
        void addComponent(Entity entity, const T &component) {
            m_addcomponentlock.Acquire();
            m_componentManager->addComponent<T>(entity, component);
            m_addcomponentlock.Release();
        }

        template<typename T>
        void removeComponent(Entity entity) {
            m_removecomponentlock.Acquire();
            m_componentManager->removeComponent<T>(entity);
            m_removecomponentlock.Release();
        }

        template<typename T>
        T &getComponent(Entity entity) {
            m_getcomponentlock.Acquire();
            T& component = m_componentManager->getComponent<T>(entity);
            m_getcomponentlock.Release();
            return component;
        }

        template<typename T>
        bool hasComponent(Entity entity) {
            m_hascomponentlock.Acquire();
            T& component = m_componentManager->hasComponent<T>(entity);
            m_hascomponentlock.Release();
            return component;
        }


    private:
        std::unique_ptr<EntityManager> m_entityManager;
        std::unique_ptr<ComponentManager> m_componentManager;
        SpinLock m_createentitylock;
        SpinLock m_destroyentitylock;
        SpinLock m_getcomponentlock;
        SpinLock m_hasomponentlock;
        SpinLock m_removecomponentlock;
        SpinLock m_addcomponentlock;
        SpinLock m_registerComponentlock;
    };
}
