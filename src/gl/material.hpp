#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <glm/glm.hpp>
#include <optional>

#include "texture.hpp"

namespace gl
{

struct Material
{
    glm::vec3 diffuse_color{1.0f, 1.0f, 1.0f};
    float alpha{1.0f};
    std::optional<Texture> diffuse_map{std::nullopt};
};

} // namespace gl

#endif // MATERIAL_HPP