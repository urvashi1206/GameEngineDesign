//
// Created by rohit on 4/7/2025.
//
#pragma once

#include "FrameInfo.hpp"
#include "ecs/ECSCoordinator.hpp"

namespace Minimal {
    class RenderSubsystem {
    public:
        explicit RenderSubsystem(ECSCoordinator &ecs);

        virtual ~RenderSubsystem();

        virtual void update(FrameInfo &frameInfo) = 0;

        virtual void render(FrameInfo &frameInfo) = 0;

    protected:
        ECSCoordinator &m_ecs;
    };
}
