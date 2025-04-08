//
// Created by rohit on 4/7/2025.
//
#pragma once

#include "FrameInfo.hpp"
#include "ecs/ECSCoordinator.hpp"

namespace Minimal {
    class System {
    public:
        explicit System(ECSCoordinator &ecs);

        virtual ~System();

        virtual void update(FrameInfo &frameInfo) = 0;

    protected:
        ECSCoordinator &m_ecs;
    };
}
