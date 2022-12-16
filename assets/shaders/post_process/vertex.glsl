#version 450 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_tex_coordinates;

out vec2 vertex_tex_coordinates;

void main()
{
    gl_Position = vec4(in_position, 1.0);
    vertex_tex_coordinates = in_tex_coordinates;
}