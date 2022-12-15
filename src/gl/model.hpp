#ifndef MODEL_HPP
#define MODEL_HPP

#include <glm/glm.hpp>

#include "mesh.hpp"

namespace gl
{

struct Model
{
    glm::vec3 scale{1.0f, 1.0f, 1.0f};
    glm::vec3 euler_angles{0.0f, 0.0f, 0.0f}; // Maybe replace this with quaternions?
    glm::vec3 translation{0.0f, 0.0f, 0.0f};
    Mesh mesh;

    glm::mat4 transform() const;
};

} // namespace gl

#endif // MODEL_HPP