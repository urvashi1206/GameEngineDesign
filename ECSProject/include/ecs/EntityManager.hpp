#pragma once

#include <cstdint>
#include <queue>

constexpr std::uint32_t MAX_ENTITIES = 5000;
using Entity = std::uint32_t;

class EntityManager {
public:
    EntityManager() {
        for (Entity e = 0; e < MAX_ENTITIES; ++e) {
            m_AvailableEntities.push(e);
        }
        m_LivingEntityCount = 0;
    }

    Entity CreateEntity() {
        Entity id = m_AvailableEntities.front();
        m_AvailableEntities.pop();
        ++m_LivingEntityCount;
        return id;
    }

    void DestroyEntity(Entity entity) {
        m_AvailableEntities.push(entity);
        --m_LivingEntityCount;
    }

private:
    std::queue<Entity> m_AvailableEntities;
    std::uint32_t m_LivingEntityCount;
};
