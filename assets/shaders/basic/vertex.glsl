#version 450 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_tex_coords;

uniform mat4 mvp = mat4(1.0);

void main()
{
    gl_Position = mvp * vec4(in_position, 1.0);
}