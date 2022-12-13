#include <numeric>

#include <glad/glad.h>

#include "mesh.hpp"

namespace gl
{

Mesh::Mesh(std::vector<float> vertices_data, std::vector<int> attributes_sizes) :
    attributes_sizes_{std::move(attributes_sizes)}, stride_{std::accumulate(attributes_sizes_.cbegin(),
                                                                            attributes_sizes_.cend(), 0)},
    number_of_vertices_{static_cast<int>(vertices_data.size()) / stride_}
{
    glGenVertexArrays(1, &vertex_array_identifier_);
    glBindVertexArray(vertex_array_identifier_);

    // Create vertex buffer, allocate memory and copy vertices data to the device
    std::uint32_t vertex_buffer_identifier{0};
    glGenBuffers(1, &vertex_buffer_identifier);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_identifier);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizei>(vertices_data.size() * sizeof(float)), vertices_data.data(),
                 GL_STATIC_DRAW);

    // Specify vertex format (default: position and texture coordinates)
    int offset{0};
    for (std::size_t index = 0; index < attributes_sizes_.size(); ++index)
    {
        glVertexAttribPointer(index, attributes_sizes_[index], GL_FLOAT, GL_FALSE, stride_ * sizeof(float),
                              reinterpret_cast<void*>(offset * sizeof(float)));
        glEnableVertexAttribArray(index);
        offset += attributes_sizes_[index];
    }
}

Mesh::Mesh(Mesh&& other) noexcept :
    number_of_vertices_{other.number_of_vertices_}, vertex_array_identifier_{other.vertex_array_identifier_}
{
    other.vertex_array_identifier_ = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
    std::swap(vertex_array_identifier_, other.vertex_array_identifier_);
    std::swap(number_of_vertices_, other.number_of_vertices_);
    return *this;
}

Mesh::~Mesh()
{
    glDeleteVertexArrays(1, &vertex_array_identifier_);
    vertex_array_identifier_ = 0;
}

void Mesh::bind()
{
    glBindVertexArray(vertex_array_identifier_);
}

void Mesh::render()
{
    bind();
    glDrawArrays(GL_TRIANGLES, 0, number_of_vertices_);
}

int Mesh::number_of_vertices() const
{
    return number_of_vertices_;
}

int Mesh::number_of_attributes() const
{
    return static_cast<int>(attributes_sizes_.size());
}

PatchMesh::PatchMesh(int vertices_per_patch, std::vector<float> vertices_data) :
    Mesh{std::move(vertices_data)}, vertices_per_patch_{vertices_per_patch}
{
    glPatchParameteri(GL_PATCH_VERTICES, vertices_per_patch_);
}

void PatchMesh::render()
{
    bind();
    glDrawArrays(GL_PATCHES, 0, number_of_vertices());
}

IndexedMesh::IndexedMesh(std::vector<float> vertices_data, std::vector<std::uint32_t> indices,
                         std::vector<int> attributes_sizes) :
    attributes_sizes_{std::move(attributes_sizes)},
    stride_{std::accumulate(attributes_sizes_.cbegin(), attributes_sizes_.cend(), 0)},
    number_of_vertices_{static_cast<int>(vertices_data.size() / stride_)}, number_of_indices_{
                                                                               static_cast<int>(indices.size())}
{
    glCreateVertexArrays(1, &vertex_array_identifier_);
    glBindVertexArray(vertex_array_identifier_);

    glGenBuffers(1, &vertex_buffer_identifier_);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_identifier_);
    glBufferData(GL_ARRAY_BUFFER, vertices_data.size() * sizeof(float), vertices_data.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &element_buffer_object_id_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object_id_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(std::uint32_t), indices.data(), GL_STATIC_DRAW);

    int offset{0};
    for (std::size_t index = 0; index < attributes_sizes_.size(); ++index)
    {
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, attributes_sizes_[index], GL_FLOAT, GL_FALSE, stride_ * sizeof(float),
                              reinterpret_cast<void*>(offset * sizeof(float)));
        offset += attributes_sizes_[index];
    }
}

IndexedMesh::IndexedMesh(IndexedMesh&& mesh) noexcept :
    attributes_sizes_{std::move(mesh.attributes_sizes_)}, stride_{mesh.stride_},
    number_of_vertices_{mesh.number_of_vertices_}, number_of_indices_{mesh.number_of_indices_},
    vertex_array_identifier_{mesh.vertex_array_identifier_}, vertex_buffer_identifier_{mesh.vertex_buffer_identifier_},
    element_buffer_object_id_{mesh.element_buffer_object_id_}
{
    mesh.number_of_vertices_ = 0;
    mesh.number_of_indices_ = 0;
    mesh.vertex_array_identifier_ = 0;
    mesh.vertex_buffer_identifier_ = 0;
    mesh.element_buffer_object_id_ = 0;
}

IndexedMesh& IndexedMesh::operator=(IndexedMesh&& mesh) noexcept
{
    std::swap(attributes_sizes_, mesh.attributes_sizes_);
    std::swap(stride_, mesh.stride_);
    std::swap(number_of_vertices_, mesh.number_of_vertices_);
    std::swap(number_of_indices_, mesh.number_of_indices_);
    std::swap(vertex_array_identifier_, mesh.vertex_array_identifier_);
    std::swap(vertex_buffer_identifier_, mesh.vertex_buffer_identifier_);
    std::swap(element_buffer_object_id_, mesh.element_buffer_object_id_);
    return *this;
}

IndexedMesh::~IndexedMesh()
{
    glDeleteBuffers(1, &element_buffer_object_id_);
    glDeleteBuffers(1, &vertex_buffer_identifier_);
    glDeleteVertexArrays(1, &vertex_array_identifier_);
}

void IndexedMesh::bind()
{
    glBindVertexArray(vertex_array_identifier_);
}

void IndexedMesh::render()
{
    bind();
    glDrawElements(GL_TRIANGLES, number_of_indices_, GL_UNSIGNED_INT, 0);
}

void IndexedMesh::update_mesh(std::vector<float> vertices_data, std::vector<std::uint32_t> indices)
{
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_identifier_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices_data.size() * sizeof(float), vertices_data.data());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object_id_);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(std::uint32_t), indices.data());
}

void IndexedMesh::update_geometry(std::vector<float> vertices_data)
{
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_identifier_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices_data.size() * sizeof(float), vertices_data.data());
}

int IndexedMesh::number_of_vertices() const
{
    return number_of_vertices_;
}

int IndexedMesh::number_of_attributes() const
{
    return static_cast<int>(attributes_sizes_.size());
}

int IndexedMesh::number_of_indices() const
{
    return number_of_indices_;
}

} // namespace gl