#version 420 core

layout (location = 0) in vec3 aPos;

out vec3 fPos;

uniform mat4 camera;

void main()
{
    fPos = aPos;
    gl_Position = (camera * vec4(aPos, 1.0)).xyww; // Push skybox to far plane
} 