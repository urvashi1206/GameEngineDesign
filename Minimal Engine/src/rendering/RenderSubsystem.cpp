//
// Created by rohit on 4/7/2025.
//
#include "RenderSubsystem.hpp"

namespace Minimal {
    RenderSubsystem::RenderSubsystem(ECSCoordinator &ecs): m_ecs(ecs) {}

    RenderSubsystem::~RenderSubsystem() = default;
}
