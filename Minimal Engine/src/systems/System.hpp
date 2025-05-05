//
// Created by rohit on 5/2/2025.
//
#pragma once
#include "ecs/ECSCoordinator.hpp"


namespace Minimal {
    class System {
    public:
        explicit System(ECSCoordinator &ecs);

        virtual ~System();

        virtual void update(float deltaTime) = 0;

    protected:
        ECSCoordinator &m_ecs;
    };
}
