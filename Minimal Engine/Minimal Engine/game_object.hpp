#pragma once
#include <memory>

#include "model.hpp"

//libs
#include <glm/gtc/matrix_transform.hpp>

namespace minimal
{
    struct transform_component
    {
        glm::vec3 translation{};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};
        glm::vec3 rotation{};

        // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix // Brendan Galea
        glm::mat4 mat4()
        {
            const float c3 = glm::cos(rotation.z);
            const float s3 = glm::sin(rotation.z);
            const float c2 = glm::cos(rotation.x);
            const float s2 = glm::sin(rotation.x);
            const float c1 = glm::cos(rotation.y);
            const float s1 = glm::sin(rotation.y);
            return glm::mat4{
                {
                    scale.x * (c1 * c3 + s1 * s2 * s3),
                    scale.x * (c2 * s3),
                    scale.x * (c1 * s2 * s3 - c3 * s1),
                    0.0f,
                },
                {
                    scale.y * (c3 * s1 * s2 - c1 * s3),
                    scale.y * (c2 * c3),
                    scale.y * (c1 * c3 * s2 + s1 * s3),
                    0.0f,
                },
                {
                    scale.z * (c2 * s1),
                    scale.z * (-s2),
                    scale.z * (c1 * c2),
                    0.0f,
                },
                {translation.x, translation.y, translation.z, 1.0f}
            };
        }
    };

    class game_object
    {
    public:
        using id_t = unsigned int;

        static game_object create()
        {
            static id_t current_id = 0;
            return game_object{current_id++};
        }

        game_object(const game_object&) = delete;
        game_object& operator=(const game_object&) = delete;
        game_object(game_object&&) = default;
        game_object& operator=(game_object&&) = default;

        id_t get_id() const { return id; }

        std::shared_ptr<model> model{};
        glm::vec3 color{};
        transform_component transform{};

    private:
        game_object(id_t obj_id) : id{obj_id}
        {
        }

        id_t id;
    };
}
