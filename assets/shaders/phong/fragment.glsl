#version 450 core

in vec3 vertex_normal;
in vec3 vertex_frag_pos;

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
uniform vec3 model_color;

void main()
{
    vec3 unit_normal = normalize(vertex_normal);
    vec3 unit_light_dir = normalize(-light.direction);

    // Ambient component
    vec3 ambient_component = light.ambient * model_color;

    // Diffuse component
    float diffuse_intensity = max(dot(unit_normal, unit_light_dir), 0.0);
    vec3 diffuse_component = diffuse_intensity * light.diffuse * model_color;

    // Specular component
    vec3 view_dir = normalize(view_pos - vertex_frag_pos);
    vec3 halfway_dir = normalize(unit_light_dir + view_dir);
    float specular_intensity = pow(max(dot(view_dir, halfway_dir), 0.0), 64.0);
    vec3 specular_component = specular_intensity * light.specular * model_color;

    frag_color = vec4(ambient_component + diffuse_component + specular_component, 1.0);
}