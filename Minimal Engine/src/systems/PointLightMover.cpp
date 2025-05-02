#include "PointLightMover.hpp"

PointLightMover::PointLightMover(Minimal::ECSCoordinator &ecs): System(ecs) {}

void PointLightMover::update(Minimal::FrameInfo &frameInfo) {
    const auto rotateLight = rotate(glm::mat4(1.0f),
                              frameInfo.frameTime,
                              {0.0f, -1.0f, 0.0f}
    );

    for (Minimal::Entity e = 0; e < m_ecs.getEntityCount(); e++) {
        if (!m_ecs.hasComponent<Minimal::PointLightComponent>(e))
            continue;

        auto &transform = m_ecs.getComponent<Minimal::TransformComponent>(e);
        // update light position
        transform.position = glm::vec3(rotateLight * glm::vec4(transform.position, 1.0f));
    }
}
