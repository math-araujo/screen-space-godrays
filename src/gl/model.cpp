#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#include "model.hpp"
#include "shader.hpp"

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
    for (auto& mesh_data : render_data_)
    {
        mesh_data.mesh.render();
    }

    for (auto& mesh_data : semitransparent_render_data_)
    {
        mesh_data.mesh.render();
    }
}

void Model::render_meshes_with_texture()
{
    if (!is_sorted_)
    {
        sort_by_texture();
    }

    for (std::size_t i = mesh_with_texture_index; i < render_data_.size(); ++i)
    {
        auto& mesh_data = render_data_[i];
        mesh_data.material.diffuse_map.value().bind(0);
        mesh_data.mesh.render();
    }
}

void Model::render_meshes_with_color(ShaderProgram& shader, const std::string& uniform_color_name)
{
    if (!is_sorted_)
    {
        sort_by_texture();
    }

    for (std::size_t i = 0; i < mesh_with_texture_index; ++i)
    {
        auto& mesh_data = render_data_[i];
        shader.set_vec3_uniform(uniform_color_name, mesh_data.material.diffuse_color);
        mesh_data.mesh.render();
    }
}

void Model::render_semitransparent_meshes(ShaderProgram& shader, const std::string& uniform_color_name)
{
    for (auto& mesh_data : semitransparent_render_data_)
    {
        const float alpha{mesh_data.material.alpha};
        shader.set_vec4_uniform(uniform_color_name, glm::vec4{mesh_data.material.diffuse_color, alpha});
        mesh_data.mesh.render();
    }
}

void Model::add_mesh_render_data(Mesh mesh, Material material)
{
    if (material.alpha == 1.0f) // Fully opaque
    {
        render_data_.emplace_back(std::move(mesh), std::move(material));
    }
    else
    {
        semitransparent_render_data_.emplace_back(std::move(mesh), std::move(material));
    }

    is_sorted_ = false;
}

std::size_t Model::number_of_meshes() const
{
    return render_data_.size();
}

void Model::sort_by_texture()
{
    std::sort(render_data_.begin(), render_data_.end(), [](const MeshRenderData& lhs, const MeshRenderData& rhs) {
        return lhs.material.diffuse_map.has_value() < rhs.material.diffuse_map.has_value();
    });
    is_sorted_ = true;
    auto first_textured_mesh =
        std::find_if(render_data_.cbegin(), render_data_.cend(),
                     [](const MeshRenderData& mesh_data) { return mesh_data.material.diffuse_map.has_value(); });
    mesh_with_texture_index = first_textured_mesh - render_data_.cbegin();
}

} // namespace gl