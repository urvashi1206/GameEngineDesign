#pragma once

#include <memory>
#include <unordered_map>
#include <typeindex>
#include <cassert>
#include "ComponentArray.hpp"

class IComponentArray {
public:
    virtual ~IComponentArray() = default;
    virtual void DestroyEntity(Entity entity) = 0;
};

template <typename T>
class ConcreteComponentArray : public IComponentArray, public ComponentArray<T> {
public:
    void DestroyEntity(Entity entity) override {
        this->DestroyEntity(entity);
    }
};
class ComponentManager {
public:
    template <typename T>
    void RegisterComponent() {
        std::type_index typeName = typeid(T);

        assert(m_ComponentArrays.find(typeName) == m_ComponentArrays.end()
            && "Registering component type more than once.");

        m_ComponentArrays[typeName] = std::make_shared<ConcreteComponentArray<T>>();
    }

    template <typename T>
    void AddComponent(Entity entity, const T& component) {
        GetComponentArray<T>()->InsertData(entity, component);
    }

    template <typename T>
    void RemoveComponent(Entity entity) {
        GetComponentArray<T>()->RemoveData(entity);
    }

    template <typename T>
    T& GetComponent(Entity entity) {
        return GetComponentArray<T>()->GetData(entity);
    }

    template <typename T>
    bool HasComponent(Entity entity) {
        return GetComponentArray<T>()->HasData(entity);
    }

    void DestroyEntity(Entity entity) {
        for (auto const& pair : m_ComponentArrays) {
            auto const& componentArray = pair.second;
            componentArray->DestroyEntity(entity);
        }
    }

private:
    std::unordered_map<std::type_index, std::shared_ptr<IComponentArray>> m_ComponentArrays{};

    template <typename T>
    std::shared_ptr<ConcreteComponentArray<T>> GetComponentArray() {
        std::type_index typeName = typeid(T);
        assert(m_ComponentArrays.find(typeName) != m_ComponentArrays.end()
            && "Component not registered before use.");
        return std::static_pointer_cast<ConcreteComponentArray<T>>(m_ComponentArrays[typeName]);
    }
};
