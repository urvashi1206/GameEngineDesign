#pragma once

#include <memory>
#include <unordered_map>
#include <typeindex>
#include <cassert>
#include "ComponentArray.hpp"

namespace Minimal {
    class IComponentArray {
    public:
        virtual ~IComponentArray() = default;

        virtual void destroyEntity(Entity entity) = 0;
    };

    template<typename T>
    class ConcreteComponentArray : public IComponentArray, public ComponentArray<T> {
    public:
        void destroyEntity(Entity entity) override {
            this->destroyEntity(entity);
        }
    };

    class ComponentManager {
    public:
        template<typename T>
        void registerComponent() {
            std::type_index typeName = typeid(T);

            assert(m_componentArrays.find(typeName) == m_componentArrays.end()
                && "Registering component type more than once.");

            m_componentArrays[typeName] = std::make_shared<ConcreteComponentArray<T> >();
        }

        template<typename T>
        void addComponent(Entity entity, const T &component) {
            getComponentArray<T>()->insertData(entity, component);
        }

        template<typename T>
        void removeComponent(Entity entity) {
            getComponentArray<T>()->RemoveData(entity);
        }

        template<typename T>
        T &getComponent(Entity entity) {
            return getComponentArray<T>()->GetData(entity);
        }

        template<typename T>
        bool hasComponent(Entity entity) {
            return getComponentArray<T>()->HasData(entity);
        }

        void DestroyEntity(Entity entity) {
            for (auto const &pair: m_componentArrays) {
                auto const &componentArray = pair.second;
                componentArray->destroyEntity(entity);
            }
        }

    private:
        std::unordered_map<std::type_index, std::shared_ptr<IComponentArray> > m_componentArrays{};

        template<typename T>
        std::shared_ptr<ConcreteComponentArray<T> > getComponentArray() {
            std::type_index typeName = typeid(T);
            assert(m_componentArrays.find(typeName) != m_componentArrays.end()
                && "Component not registered before use.");
            return std::static_pointer_cast<ConcreteComponentArray<T> >(m_componentArrays[typeName]);
        }
    };
}
