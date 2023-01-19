#version 450 core

in vec3 vertex_frag_pos;
in vec3 vertex_normal;
in vec2 vertex_tex_coordinates;

out vec4 frag_color;

struct Light
{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Light light;
uniform vec3 view_pos;
layout (binding = 0) uniform sampler2D diffuse_texture;

void main()
{
    vec3 unit_normal = normalize(vertex_normal);
    vec3 unit_light_dir = normalize(-light.direction);
    vec3 diffuse_color = texture(diffuse_texture, vertex_tex_coordinates).rgb;

    // Ambient component
    vec3 ambient_component = light.ambient * diffuse_color;

    // Diffuse component
    float diffuse_intensity = max(dot(unit_normal, unit_light_dir), 0.0);
    vec3 diffuse_component = diffuse_intensity * light.diffuse * diffuse_color;

    // Specular component
    vec3 view_dir = normalize(view_pos - vertex_frag_pos);
    vec3 halfway_dir = normalize(unit_light_dir + view_dir);
    float specular_intensity = pow(max(dot(view_dir, halfway_dir), 0.0), 64.0);
    vec3 specular_component = specular_intensity * light.specular * diffuse_color;

    vec3 color = ambient_component + diffuse_component + specular_component;
    vec3 gamma_corrected_color = pow(color, vec3(1.0 / 2.0));
    frag_color = vec4(gamma_corrected_color, 1.0);
}