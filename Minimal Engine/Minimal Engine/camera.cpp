#include "camera.hpp"

//std
#include <cassert>
#include <limits>

namespace minimal
{
    void camera::set_othrographic_projection(float left, float right, float top, float bottom, float near, float far)
    {
        projection_matrix_ = glm::mat4{1.0f};
        projection_matrix_[0][0] = 2.f / (right - left);
        projection_matrix_[1][1] = 2.f / (bottom - top);
        projection_matrix_[2][2] = 1.f / (far - near);
        projection_matrix_[3][0] = -(right + left) / (right - left);
        projection_matrix_[3][1] = -(bottom + top) / (bottom - top);
        projection_matrix_[3][2] = -near / (far - near);
    }

    void camera::set_perspective_projection(float fovy, float aspect, float near, float far)
    {
        assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
        const float tanHalfFovy = tan(fovy / 2.f);
        projection_matrix_ = glm::mat4{0.0f};
        projection_matrix_[0][0] = 1.f / (aspect * tanHalfFovy);
        projection_matrix_[1][1] = 1.f / (tanHalfFovy);
        projection_matrix_[2][2] = far / (far - near);
        projection_matrix_[2][3] = 1.f;
        projection_matrix_[3][2] = -(far * near) / (far - near);
    }
}
