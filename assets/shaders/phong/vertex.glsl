#version 450 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_tex_coordinates;

uniform mat4 model = mat4(1.0);
uniform mat4 mvp = mat4(1.0);

out vec3 vertex_frag_pos;
out vec3 vertex_normal;
out vec2 vertex_tex_coordinates;

void main()
{
    vertex_normal = in_normal;
    vertex_tex_coordinates = in_tex_coordinates;
    vec4 homogeneous_position = vec4(in_position, 1.0);
    vertex_frag_pos = vec3(model * homogeneous_position);
    gl_Position = mvp * homogeneous_position;
}