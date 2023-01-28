#version 450 core

layout (location = 0) in vec3 in_vertex_coordinates;

uniform mat4 light_space_transform;
uniform mat4 model;

void main()
{
    gl_Position = light_space_transform * model * vec4(in_vertex_coordinates, 1.0);
}