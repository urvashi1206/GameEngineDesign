#pragma once

#include <cstdint>
#include <queue>

constexpr std::uint32_t MAX_ENTITIES = 5000;
using Entity = std::uint32_t;

/**
 * Responsible for creating and destroying entities.
 */
class EntityManager {
public:
    EntityManager() {
        // Initialize the queue with all possible entity IDs.
        for (Entity e = 0; e < MAX_ENTITIES; ++e) {
            m_AvailableEntities.push(e);
        }
        m_LivingEntityCount = 0;
    }

    Entity CreateEntity() {
        // Reuse an ID from the front of the queue
        Entity id = m_AvailableEntities.front();
        m_AvailableEntities.pop();
        ++m_LivingEntityCount;
        return id;
    }

    void DestroyEntity(Entity entity) {
        // Return the entity ID to the queue
        m_AvailableEntities.push(entity);
        --m_LivingEntityCount;
    }

private:
    std::queue<Entity> m_AvailableEntities;  // Queue of unused entity IDs
    std::uint32_t m_LivingEntityCount;       // How many entities are currently in use
};
