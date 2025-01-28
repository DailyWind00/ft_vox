#version 460 core

layout (location = 0) in ivec3 blockData;
layout (std140, binding = 0) buffer SSBO {
	ivec4 worldpos[]; // Fuck you khronos
};

uniform mat4 transform; // Projection * View * Model

void main() {
	ivec3 worldOffset = worldpos[gl_DrawID].xyz * 32;
	gl_Position = transform * vec4(worldOffset + blockData, 1.0);
}