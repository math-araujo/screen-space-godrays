#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <glm/glm.hpp>

namespace gl
{

struct DirectionalLight
{
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct PointLight
{
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

} // namespace gl

#endif // LIGHT_HPP