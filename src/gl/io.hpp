#ifndef IO_HPP
#define IO_HPP

#include <string>
#include <unordered_map>

#include "mesh.hpp"
#include "model.hpp"

namespace gl
{

// std::unordered_map<std::string, Mesh> read_triangle_mesh(const std::string& filename, bool verbose = false);
std::unordered_map<std::string, Model> read_triangle_mesh(const std::string& filename, bool verbose = false);

} // namespace gl

#endif // IO_HPP