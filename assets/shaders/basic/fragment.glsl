#version 450 core

out vec4 frag_color;

uniform vec4 color = vec4(1.0);

void main()
{
    vec3 gamma_corrected_color = pow(color.rgb, vec3(1.0 / 2.2));
    frag_color = vec4(gamma_corrected_color, color.a);
}