#version 330 core

in float pass_radius;
flat in int pass_type;

out vec4 fragColor;

void main()
{
    float r = 1;
    float b = 0;
    if (pass_type == 3) {
        r = 0;
        b = 0.5;
    }
    if (pass_type == 4) {
        r = 1;
        b = 1;
    }
    fragColor = vec4(r, pass_radius, b, 1);
}
