#include <array>
#include <iostream>
#include <tiny_obj_loader.h>

#include "io.hpp"

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
    }

    std::unordered_map<std::string, Model> models;
    std::cout << "Number of shapes = " << shapes.size() << std::endl;
    for (std::size_t shape_index = 0; const auto& shape : shapes)
    {
        std::vector<float> vertices_data;
        bool has_normals{false};
        bool has_tex_coords{false};
        std::size_t index_offset{0};

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

            if (!materials.empty())
            {
                const auto& current_material = materials[shape.mesh.material_ids[face_index]];

                bool new_mesh{false};
                if (previous_material_name != current_material.name)
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
                    // TODO:
                    // 1) Create Material here
                    // 2) Create Model from vertices + Material
                    // 3) Emplace model at container
                }

                previous_material_name = current_material.name;
            }

            index_offset += verts_per_face;
            ++face_index;
        }

        std::vector<int> attributes_sizes{3};
        if (has_normals)
        {
            attributes_sizes.emplace_back(3);
        }
        if (has_tex_coords)
        {
            attributes_sizes.emplace_back(2);
        }

        Mesh mesh{vertices_data, attributes_sizes};
        Model model;
        model.meshes.emplace_back(std::move(mesh));
        models.emplace(shape.name, std::move(model));
    }

    return models;
}

} // namespace gl