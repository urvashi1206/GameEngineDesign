//
// Created by rohit on 4/7/2025.
//

#include "System.hpp"

namespace Minimal {
    System::System(ECSCoordinator &ecs)
        : m_ecs(ecs) {
    }

    System::~System() = default;
}
