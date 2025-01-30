#version 460 core

layout (location = 0) in uvec2 blockData;
layout (std140, binding = 0) buffer SSBO {
	ivec4 worldpos[]; // Fuck you khronos
};

flat out ivec3	geoPos;

void main() {
	// Decode blockData bitmask :
	uvec3 position;
	position.x = (blockData.x)       & 0x1F;
	position.y = (blockData.x >> 5)  & 0x1F;
	position.z = (blockData.x >> 10) & 0x1F;

	ivec3 worldOffset = worldpos[gl_DrawID].xyz * 32;

	geoPos = ivec3(position);
	gl_Position = vec4(worldOffset + geoPos , 1.0);
}
