#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace minimal
{
    class camera
    {
    public:
        void set_othrographic_projection(float left, float right, float top, float bottom, float near, float far);
        void set_perspective_projection(float fovy, float aspect, float near, float far);

        const glm::mat4& get_projection() const { return projection_matrix_; }

    private:
        glm::mat4 projection_matrix_{1.0f};
    };
    
}
