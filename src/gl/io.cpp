#include <algorithm>
#include <array>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <numeric>
#include <tiny_obj_loader.h>

#include "io.hpp"
#include "material.hpp"
#include "texture.hpp"

namespace gl
{

// std::unordered_map<std::string, Mesh> read_triangle_mesh(const std::string& filename, bool verbose)
std::unordered_map<std::string, Model> read_triangle_mesh(const std::string& filename, bool verbose)
{
    // Cannot concatenate std::string_view, must use std::string
    const static std::string base_materials_path{"assets/materials/"};
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = base_materials_path;
    tinyobj::ObjReader reader;

    const static std::string base_models_path{"assets/models/"};
    if (!reader.ParseFromFile(base_models_path + filename, reader_config))
    {
        if (!reader.Error().empty())
        {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty())
    {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    if (verbose)
    {
        std::cout << "Number of materials: " << materials.size() << "\n";
        for (auto& material : materials)
        {
            std::cout << "\tMaterial name: " << material.name << std::endl;
            std::cout << "\tDiffuse texture name: " << material.diffuse_texname << std::endl;
        }

        std::cout << "Number of shapes = " << shapes.size() << std::endl;
    }

    std::unordered_map<std::string, Model> models;
    for (std::size_t shape_index = 0; const auto& shape : shapes)
    {
        if (verbose)
        {
            std::cout << "Shape " << shape_index << ":\n\tName: " << shape.name << std::endl;
        }

        std::vector<float> vertices_data;
        bool has_normals{false};
        bool has_tex_coords{false};
        std::size_t index_offset{0};
        Model model;

        std::string previous_material_name{materials.empty() ? "" : materials[shape.mesh.material_ids[0]].name};
        for (std::size_t face_index = 0; const std::size_t verts_per_face : shape.mesh.num_face_vertices)
        {
            for (std::size_t vertex_index = 0; vertex_index < verts_per_face; ++vertex_index)
            {
                const tinyobj::index_t index{shape.mesh.indices[index_offset + vertex_index]};

                for (int i = 0; i < 3; ++i)
                {
                    vertices_data.emplace_back(attrib.vertices[3 * index.vertex_index + i]);
                }

                if (index.normal_index >= 0)
                {
                    has_normals = true;
                    for (int i = 0; i < 3; ++i)
                    {
                        vertices_data.emplace_back(attrib.normals[3 * index.normal_index + i]);
                    }
                }

                if (index.texcoord_index >= 0)
                {
                    has_tex_coords = true;
                    for (int i = 0; i < 2; ++i)
                    {
                        vertices_data.emplace_back(attrib.texcoords[2 * index.texcoord_index + i]);
                    }
                }
            }

            std::string new_material_name;
            if (!materials.empty())
            {
                const auto& current_material = materials[shape.mesh.material_ids[face_index]];
                new_material_name = current_material.name;

                /* A new mesh is detected when a face with a new material is found,
                or when we reached the last face of the triangle mesh. If the former
                case, the last face read from the file belongs to the next mesh,
                not to the current mesh, and must be saved for usage later.
                */
                bool new_mesh{false};
                bool save_last_face{false};
                if (previous_material_name != new_material_name)
                {
                    new_mesh = true;
                    save_last_face = true;
                }
                else if (face_index == shape.mesh.num_face_vertices.size() - 1)
                {
                    new_mesh = true;
                }

                if (new_mesh)
                {
                    const std::size_t previous_material_index{face_index == 0 ? 0 : face_index - 1};
                    const auto& previous_material = materials[shape.mesh.material_ids[previous_material_index]];
                    std::vector<int> attributes_sizes{3};
                    if (has_normals)
                    {
                        attributes_sizes.emplace_back(3);
                    }
                    if (has_tex_coords)
                    {
                        attributes_sizes.emplace_back(2);
                    }

                    std::vector<float> last_face;
                    if (save_last_face)
                    {
                        const int num_attributes{
                            std::accumulate(attributes_sizes.cbegin(), attributes_sizes.cend(), 0)};
                        const std::size_t num_elements{verts_per_face * num_attributes};
                        last_face.reserve(num_elements);
                        last_face.insert(last_face.end(), vertices_data.end() - num_elements, vertices_data.end());
                        for (std::size_t i = 0; i < num_elements; ++i)
                        {
                            vertices_data.pop_back();
                        }
                    }
                    Mesh mesh{std::move(vertices_data), std::move(attributes_sizes)};
                    vertices_data.clear();
                    vertices_data = std::move(last_face);

                    Material material;
                    if (previous_material.diffuse_texname.empty())
                    {
                        const glm::vec3 diffuse_color{glm::make_vec3(previous_material.diffuse)};
                        material.diffuse_color = diffuse_color;
                        material.alpha = previous_material.dissolve;
                    }
                    else
                    {
                        const static std::string textures_path{"assets/textures/"};
                        material.diffuse_map =
                            create_texture_from_file(textures_path + previous_material.diffuse_texname,
                                                     Texture::Attributes{.internal_format = GL_SRGB});
                    }
                    model.add_mesh_render_data(std::move(mesh), std::move(material));
                }
            }

            previous_material_name = new_material_name;
            index_offset += verts_per_face;
            ++face_index;
        }

        /*
        If there was no material on the OBJ file, there will be no split based on
        materials. Hence the vertices_data will not be empty and there will be a single
        mesh.
        */
        if (!vertices_data.empty())
        {
            assert(materials.empty());
            std::vector<int> attributes_sizes{3};
            if (has_normals)
            {
                attributes_sizes.emplace_back(3);
            }
            if (has_tex_coords)
            {
                attributes_sizes.emplace_back(2);
            }

            Mesh mesh{std::move(vertices_data), std::move(attributes_sizes)};
            vertices_data.clear();
            model.add_mesh_render_data(std::move(mesh), Material{});
        }

        if (verbose)
        {
            std::cout << "Model with " << model.number_of_meshes() << " meshes" << std::endl;
        }

        models.emplace(shape.name, std::move(model));
    }

    return models;
}

} // namespace gl