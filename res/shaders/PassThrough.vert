#version 330 core

uniform mat4 projMatrix;
uniform mat4 viewMatrix;

layout(location = 0) in vec3 position;
layout(location = 1) in float radius;
layout(location = 2) in int type;

out float pass_radius;
flat out int pass_type;

void main()
{
	pass_radius = radius;
	pass_type = type;

    gl_Position = projMatrix * viewMatrix * vec4(position, 1);
}
