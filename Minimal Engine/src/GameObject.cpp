#include "GameObject.hpp"

namespace Minimal {
    GameObject GameObject::makePointLight(float intensity, float radius, glm::vec3 color) {
        auto lightObject = create();
        lightObject.color = color;
        lightObject.transform.scale.x = radius;
        lightObject.pointLight = std::make_unique<PointLightComponent>();
        lightObject.pointLight->lightIntensity = intensity;

        return lightObject;
    }
}
