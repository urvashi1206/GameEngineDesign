#pragma once

#include <array>
#include <unordered_map>
#include <cassert>
#include "EntityManager.hpp"

/**
 * A dense array of components, mapping Entity IDs to component instances.
 */
template <typename T>
class ComponentArray {
public:
    void InsertData(Entity entity, const T& component) {
        // Ensure the entity does not already have this component
        assert(m_EntityToIndexMap.find(entity) == m_EntityToIndexMap.end()
            && "Component added to same entity more than once.");

        size_t newIndex = m_Size;
        m_EntityToIndexMap[entity] = newIndex;
        m_IndexToEntityMap[newIndex] = entity;
        m_ComponentArray[newIndex] = component;
        ++m_Size;
    }

    void RemoveData(Entity entity) {
        // Ensure the entity currently has this component
        assert(m_EntityToIndexMap.find(entity) != m_EntityToIndexMap.end()
            && "Removing non-existent component.");

        // Swap & pop technique for O(1) removal
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
        // Ensure the entity currently has this component
        assert(m_EntityToIndexMap.find(entity) != m_EntityToIndexMap.end()
            && "Retrieving non-existent component.");
        return m_ComponentArray[m_EntityToIndexMap[entity]];
    }

    bool HasData(Entity entity) const {
        return (m_EntityToIndexMap.find(entity) != m_EntityToIndexMap.end());
    }

    // Optional: Provide size/usage info
    size_t Size() const { return m_Size; }

private:
    // Packed array of components
    std::array<T, MAX_ENTITIES> m_ComponentArray{};

    // Entity-to-index lookup
    std::unordered_map<Entity, size_t> m_EntityToIndexMap{};

    // Index-to-entity lookup
    std::unordered_map<size_t, Entity> m_IndexToEntityMap{};

    size_t m_Size = 0;  // How many valid entries are in the array
};
