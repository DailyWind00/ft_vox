#version 460 core

layout (location = 0) in uint blockData;
layout (std140, binding = 0) buffer SSBO {
	ivec4 worldpos[]; // Fuck you khronos
};

uniform mat4 transform; // Projection * View * Model

void main() {
	// Decode blockData bitmask :
	uvec3 position;
	position.x = (blockData)       & 0x1F;
	position.y = (blockData >> 5)  & 0x1F;
	position.z = (blockData >> 10) & 0x1F;

	ivec3 worldOffset = worldpos[gl_DrawID].xyz * 32;
	gl_Position = transform * vec4(worldOffset + ivec3(position), 1.0);
}