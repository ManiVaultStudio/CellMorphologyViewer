#version 330 core

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

layout(location = 0) in vec3 position;
layout(location = 1) in float radius;
layout(location = 2) in int type;

out float pass_radius;
out vec3 pass_color;

void main()
{
    pass_radius = radius;
    
    vec3 color = vec3(1.0, 1.0, 1.0);

    // Soma
    if (type == 1)
    {
        color = vec3(0.706, 0.22, 0.38);
    }
    // Axon
    else if (type == 2)
    {
        color = vec3(0, 60/255.0, 125/255.0);
    }
    // Basal dendrite
    else if (type == 3)
    {
        color = vec3(0.84, 0.54, 0.21);
    }
    // Apical dendrite
    else if (type == 4)
    {
        color = vec3(0.706, 0.22, 0.38);
    }

    pass_color = color;

    gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(position, 1);
}
