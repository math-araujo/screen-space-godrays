#include <array>
#include <iostream>
#include <tiny_obj_loader.h>

#include "io.hpp"

namespace gl
{

std::unordered_map<std::string, Mesh> read_triangle_mesh(const std::string& filename, bool verbose)
{
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "assets/";
    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(filename.data(), reader_config))
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
    std::unordered_map<std::string, Mesh> meshes;
    for (std::size_t shape_index = 0; const auto& shape : shapes)
    {
        std::vector<float> vertices_data;
        bool has_normals{false};
        bool has_tex_coords{false};
        std::size_t index_offset{0};

        for (const std::size_t verts_per_face : shape.mesh.num_face_vertices)
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

            index_offset += verts_per_face;
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
        meshes.emplace(shape.name, Mesh(vertices_data, attributes_sizes));
    }

    return meshes;
}

} // namespace gl