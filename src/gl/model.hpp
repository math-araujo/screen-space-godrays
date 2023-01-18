#ifndef MODEL_HPP
#define MODEL_HPP

#include <glm/glm.hpp>

#include "material.hpp"
#include "mesh.hpp"
#include "texture.hpp"

namespace gl
{

struct MeshRenderData
{
    Mesh mesh;
    Material material;

    MeshRenderData(Mesh mesh, Material material) : mesh{std::move(mesh)}, material{std::move(material)}
    {
    }
};

struct Model
{
    glm::vec3 scale{1.0f, 1.0f, 1.0f};
    glm::vec3 euler_angles{0.0f, 0.0f, 0.0f}; // Maybe replace this with quaternions?
    glm::vec3 translation{0.0f, 0.0f, 0.0f};
    std::vector<MeshRenderData> render_data;
    // std::vector<MeshRenderData> opaque_render_data; // TODO
    // std::vector<MeshRenderData> semitransparent_render_data; // TODO

    glm::mat4 transform() const;
    void render();
};

} // namespace gl

#endif // MODEL_HPP