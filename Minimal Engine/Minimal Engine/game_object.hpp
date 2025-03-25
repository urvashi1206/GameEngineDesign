#pragma once
#include <memory>

#include "model.hpp"

//libs
#include <unordered_map>
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
        glm::mat4 mat4();
        
        glm::mat3 normal_matrix(); 
    };

    class game_object
    {
    public:
        using id_t = unsigned int;
        using map = std::unordered_map<id_t, game_object>;

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
