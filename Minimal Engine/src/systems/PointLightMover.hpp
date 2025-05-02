#pragma once
#include "System.hpp"

class PointLightMover final : public Minimal::System {
public:
    explicit PointLightMover(Minimal::ECSCoordinator &ecs);

    void update(Minimal::FrameInfo &frameInfo) override;
};
