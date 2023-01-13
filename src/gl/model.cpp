#include "model.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace gl
{

glm::mat4 Model::transform() const
{
    glm::mat4 transform_matrix = glm::translate(glm::mat4{1.0f}, translation);
    transform_matrix = glm::rotate(transform_matrix, euler_angles.y, glm::vec3{0.0f, 1.0f, 0.0f});
    transform_matrix = glm::rotate(transform_matrix, euler_angles.x, glm::vec3{1.0f, 0.0f, 0.0f});
    transform_matrix = glm::rotate(transform_matrix, euler_angles.z, glm::vec3{0.0f, 0.0f, 1.0f});
    transform_matrix = glm::scale(transform_matrix, scale);
    return transform_matrix;
}

void Model::render()
{
    for (auto& mesh : meshes)
    {
        mesh.render();
    }
}

} // namespace gl