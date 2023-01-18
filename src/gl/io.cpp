#include <algorithm>
#include <array>
#include <iostream>
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
    int total_mesh_count{0};
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

                bool new_mesh{false};
                if (previous_material_name != new_material_name)
                {
                    new_mesh = true;
                }
                else if (face_index == shape.mesh.num_face_vertices.size() - 1)
                {
                    previous_material_name = current_material.name;
                    new_mesh = true;
                }

                if (new_mesh)
                {
                    const std::size_t previous_material_index{std::max<std::size_t>(face_index - 1, 0)};
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

                    Mesh mesh{std::move(vertices_data), std::move(attributes_sizes)};
                    vertices_data.clear();

                    if (previous_material.diffuse_texname.empty())
                    {
                        const glm::vec3 diffuse_color{previous_material.diffuse[0], previous_material.diffuse[1],
                                                      previous_material.diffuse[2]};
                        Material material{.diffuse_color{diffuse_color}, .alpha{previous_material.dissolve}};
                        // model.render_data.emplace_back(std::move(mesh), std::move(material));
                        model.add_mesh_render_data(std::move(mesh), std::move(material));
                    }
                    else
                    {
                        const static std::string textures_path{"assets/textures/"};
                        Material material{
                            .diffuse_map{create_texture_from_file(textures_path + previous_material.diffuse_texname)}};
                        // model.render_data.emplace_back(std::move(mesh), std::move(material));
                        model.add_mesh_render_data(std::move(mesh), std::move(material));
                    }
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
            // model.render_data.emplace_back(std::move(mesh), Material{});
            model.add_mesh_render_data(std::move(mesh), Material{});
        }
        std::cout << "Model with " << model.number_of_meshes() << " meshes" << std::endl;
        models.emplace(shape.name, std::move(model));
    }

    return models;
}

} // namespace gl