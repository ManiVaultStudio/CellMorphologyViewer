#version 330 core

in vec3 pass_color;

out vec4 fragColor;

void main()
{
    fragColor = vec4(pass_color, 1);
}
