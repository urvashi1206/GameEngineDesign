#pragma once

#include <array>
#include <cassert>
#include <unordered_map>

#include "EntityManager.hpp"

namespace Minimal {
    template<typename T>
    class ComponentArray {
    public:
        void insertData(Entity entity, const T &component) {
            assert(m_entityToIndexMap.find(entity) == m_entityToIndexMap.end()
                && "Component added to same entity more than once.");

            size_t newIndex = m_size;
            m_entityToIndexMap[entity] = newIndex;
            m_indexToEntityMap[newIndex] = entity;
            m_componentArray[newIndex] = component;
            ++m_size;
        }

        void removeData(Entity entity) {
            assert(m_entityToIndexMap.find(entity) != m_entityToIndexMap.end()
                && "Removing non-existent component.");

            size_t indexOfRemoved = m_entityToIndexMap[entity];
            size_t indexOfLast = m_size - 1;
            m_componentArray[indexOfRemoved] = m_componentArray[indexOfLast];

            Entity entityOfLast = m_indexToEntityMap[indexOfLast];
            m_entityToIndexMap[entityOfLast] = indexOfRemoved;
            m_indexToEntityMap[indexOfRemoved] = entityOfLast;

            m_entityToIndexMap.erase(entity);
            m_indexToEntityMap.erase(indexOfLast);

            --m_size;
        }

        T &getData(Entity entity) {
            assert(m_entityToIndexMap.find(entity) != m_entityToIndexMap.end()
                && "Retrieving non-existent component.");
            return m_componentArray[m_entityToIndexMap[entity]];
        }

        bool hasData(Entity entity) const {
            return (m_entityToIndexMap.find(entity) != m_entityToIndexMap.end());
        }

        void destroyEntity(Entity entity) {
            if (hasData(entity)) {
                removeData(entity);
            }
        }

        size_t size() const { return m_size; }

    private:
        std::array<T, MAX_ENTITIES> m_componentArray{};
        std::unordered_map<Entity, size_t> m_entityToIndexMap{};
        std::unordered_map<size_t, Entity> m_indexToEntityMap{};
        size_t m_size = 0;
    };
}
