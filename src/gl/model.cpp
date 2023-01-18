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
    for (std::size_t i = 0; auto& [mesh, material] : render_data)
    {
        if (material.diffuse_map)
        {
            material.diffuse_map.value().bind(0);
        }
        else
        {
            continue; // TODO: Fix rendering of non-textured meshes
        }

        mesh.render();
        ++i;
    }
}

} // namespace gl