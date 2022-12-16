#version 450 core

out vec4 frag_color;

uniform float alpha = 0.3;

void main()
{
    frag_color = vec4(1.0, 0.0, 0.0, alpha);
}