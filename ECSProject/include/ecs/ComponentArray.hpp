#pragma once

#include <array>
#include <unordered_map>
#include <cassert>
#include "EntityManager.hpp"

template <typename T>
class ComponentArray {
public:
    void InsertData(Entity entity, const T& component) {
        assert(m_EntityToIndexMap.find(entity) == m_EntityToIndexMap.end()
            && "Component added to same entity more than once.");

        size_t newIndex = m_Size;
        m_EntityToIndexMap[entity] = newIndex;
        m_IndexToEntityMap[newIndex] = entity;
        m_ComponentArray[newIndex] = component;
        ++m_Size;
    }

    void RemoveData(Entity entity) {
        assert(m_EntityToIndexMap.find(entity) != m_EntityToIndexMap.end()
            && "Removing non-existent component.");

        size_t indexOfRemoved = m_EntityToIndexMap[entity];
        size_t indexOfLast = m_Size - 1;
        m_ComponentArray[indexOfRemoved] = m_ComponentArray[indexOfLast];

        Entity entityOfLast = m_IndexToEntityMap[indexOfLast];
        m_EntityToIndexMap[entityOfLast] = indexOfRemoved;
        m_IndexToEntityMap[indexOfRemoved] = entityOfLast;

        m_EntityToIndexMap.erase(entity);
        m_IndexToEntityMap.erase(indexOfLast);

        --m_Size;
    }

    T& GetData(Entity entity) {
        assert(m_EntityToIndexMap.find(entity) != m_EntityToIndexMap.end()
            && "Retrieving non-existent component.");
        return m_ComponentArray[m_EntityToIndexMap[entity]];
    }

    bool HasData(Entity entity) const {
        return (m_EntityToIndexMap.find(entity) != m_EntityToIndexMap.end());
    }

    void DestroyEntity(Entity entity) {
        if (HasData(entity)) {
            RemoveData(entity);
        }
    }

    size_t Size() const { return m_Size; }

private:
    std::array<T, MAX_ENTITIES> m_ComponentArray{};
    std::unordered_map<Entity, size_t> m_EntityToIndexMap{};
    std::unordered_map<size_t, Entity> m_IndexToEntityMap{};
    size_t m_Size = 0;
};
