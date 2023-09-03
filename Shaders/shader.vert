#version 450

layout(location = 0) in vec2 vertPosition;
layout(location = 1) in vec3 vertColour;

layout(location = 0) out vec3 fragColour;

void main()
{
    gl_Position = vec4(vertPosition, 0.0, 1.0);
    fragColour = vertColour;
}