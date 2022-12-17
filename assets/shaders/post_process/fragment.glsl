#version 450 core

in vec2 vertex_tex_coordinates;
out vec4 frag_color;

layout (binding = 0) uniform sampler2D occlusion_map_sampler;
uniform float alpha = 0.3;
uniform vec4 screen_space_light_pos;

struct PostprocessingCoefficients
{
    int num_samples;
    float density;
    float exposure;
    float decay;
    float weight;
};

uniform bool apply_radial_blur = true;
uniform PostprocessingCoefficients coefficients;

vec4 radial_blur(PostprocessingCoefficients coefficients, vec2 screen_space_position);

void main()
{
    frag_color = vec4(0.0);
    if (apply_radial_blur)
    {
        frag_color = radial_blur(coefficients, screen_space_light_pos.xy);
    }
    else
    {
        frag_color = texture(occlusion_map_sampler, vertex_tex_coordinates);
    }
}

vec4 radial_blur(PostprocessingCoefficients coefficients, vec2 screen_space_position)
{
    vec2 delta_tex_coord = (vertex_tex_coordinates - screen_space_position) * coefficients.density * (1.0 / float(coefficients.num_samples));
    vec2 tex_coordinates = vertex_tex_coordinates;
    vec3 color = texture(occlusion_map_sampler, tex_coordinates).rgb;
    float decay = 1.0;
    for (int i = 0; i < coefficients.num_samples; ++i)
    {
        tex_coordinates -= delta_tex_coord;
        vec3 current_sample = texture(occlusion_map_sampler, tex_coordinates).rgb;
        current_sample *= decay * coefficients.weight;
        color += current_sample;
        decay *= coefficients.decay;
    }

    return vec4(color * coefficients.exposure, 1.0);
}