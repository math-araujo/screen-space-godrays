#version 450 core

in vec3 vertex_frag_pos;
in vec3 vertex_normal;
in vec2 vertex_tex_coordinates;
in vec4 vertex_frag_pos_light_space;

out vec4 frag_color;

struct DirectionalLight
{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform DirectionalLight light;
uniform vec3 view_pos;
uniform float bias;

#ifdef DIFFUSE_MAP
layout (binding = 0) uniform sampler2D diffuse_map;
#else
uniform vec3 diffuse_color = vec3(1.0, 0.0, 0.0);
#endif
layout (binding = 1) uniform sampler2D shadow_map;

float shadow_intensity(vec4 frag_pos_light_space, vec3 unit_normal, vec3 unit_light_dir)
{
    vec3 ndc_coordinates = frag_pos_light_space.xyz / frag_pos_light_space.w;
    vec3 depth_space_coordinates = (0.5 * ndc_coordinates) + 0.5;
    if (depth_space_coordinates.z > 1.0)
    {
        return 0.0;
    }

    float current_depth = depth_space_coordinates.z;
    float shadow = 0.0;
    vec2 texel_size = 1.0 / textureSize(shadow_map, 0);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcf_closest_depth = texture(shadow_map, depth_space_coordinates.xy + vec2(x, y) * texel_size).r;
            shadow += current_depth - bias > pcf_closest_depth ? 1.0 : 0.0;
        }
    }

    shadow /= 9.0;
    return shadow;
}

void main()
{
    vec3 unit_normal = normalize(vertex_normal);
    vec3 unit_light_dir = normalize(light.direction);

    #ifdef DIFFUSE_MAP
    vec3 diffuse_color = texture(diffuse_map, vertex_tex_coordinates).rgb;
    #endif

    // Ambient component
    vec3 ambient_component = light.ambient * diffuse_color;

    // Diffuse component
    float diffuse_intensity = max(dot(unit_normal, unit_light_dir), 0.0);
    vec3 diffuse_component = diffuse_intensity * light.diffuse * diffuse_color;

    // Specular component
    vec3 view_dir = normalize(view_pos - vertex_frag_pos);
    vec3 halfway_dir = normalize(unit_light_dir + view_dir);
    float specular_intensity = pow(max(dot(view_dir, halfway_dir), 0.0), 64.0);
    vec3 specular_component = specular_intensity * light.specular * vec3(0.1f);

    float shadow = shadow_intensity(vertex_frag_pos_light_space, unit_normal, normalize(light.direction - vertex_frag_pos));
    vec3 color = ambient_component + ((1.0f - shadow) * (diffuse_component + specular_component));
    vec3 gamma_corrected_color = pow(color, vec3(1.0 / 2.0));
    frag_color = vec4(gamma_corrected_color, 1.0);
}