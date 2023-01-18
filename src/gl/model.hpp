#ifndef MODEL_HPP
#define MODEL_HPP

#include <glm/glm.hpp>
#include <string>

#include "material.hpp"
#include "mesh.hpp"
#include "texture.hpp"

namespace gl
{

// Forward declaration
class ShaderProgram;

struct MeshRenderData
{
    Mesh mesh;
    Material material;

    MeshRenderData(Mesh mesh, Material material) : mesh{std::move(mesh)}, material{std::move(material)}
    {
    }
};

class Model
{
public:
    glm::vec3 scale{1.0f, 1.0f, 1.0f};
    glm::vec3 euler_angles{0.0f, 0.0f, 0.0f}; // Maybe replace this with quaternions?
    glm::vec3 translation{0.0f, 0.0f, 0.0f};

    glm::mat4 transform() const;
    void render();
    void render_meshes_with_texture();
    void render_meshes_with_color(ShaderProgram& shader, const std::string& uniform_color_name);
    void add_mesh_render_data(Mesh mesh, Material material);
    std::size_t number_of_meshes() const;

    /*
    Sorts render data by textured and non-textured meshes.
    This allows to render meshes in batches using a specific
    shader for textured meshes and a different one for
    non-textured meshes, reducing shader binding calls.
    */
    void sort_by_texture();

private:
    std::vector<MeshRenderData> render_data_;
    bool is_sorted_{false};
    std::size_t mesh_with_texture_index{0};
    // std::vector<MeshRenderData> opaque_render_data; // TODO
    // std::vector<MeshRenderData> semitransparent_render_data; // TODO
};

} // namespace gl

#endif // MODEL_HPP