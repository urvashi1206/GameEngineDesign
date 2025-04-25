#pragma once

#include <cstdint>
#include <queue>

namespace Minimal {
    constexpr std::uint32_t MAX_ENTITIES = 5000;
    using Entity = std::uint32_t;

    class EntityManager {
    public:
        EntityManager() {
            for (Entity e = 0; e < MAX_ENTITIES; ++e) {
                m_availableEntities.push(e);
            }
            m_livingEntityCount = 0;
        }

        Entity createEntity() {
            Entity id = m_availableEntities.front();
            m_availableEntities.pop();
            ++m_livingEntityCount;
            return id;
        }

        void destroyEntity(Entity entity) {
            m_availableEntities.push(entity);
            --m_livingEntityCount;
        }

        uint32_t getEntityCount() const {
            return m_livingEntityCount;
        }

    private:
        std::queue<Entity> m_availableEntities;
        std::uint32_t m_livingEntityCount;
    };
}
