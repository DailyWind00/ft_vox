#version 460 core

struct Chunk {
	ivec4	worldpos; // Fuck you khronos
};

layout (location = 0) in uvec2 blockData;
layout (std430, binding = 0) buffer SSBO {
	Chunk chunks[];
};

flat out uint faces;

void main() {
	// Decode blockData bitmask :
	uvec3	position;
	position.x = (blockData.x)       & 0x1F;
	position.y = (blockData.x >> 5)  & 0x1F;
	position.z = (blockData.x >> 10) & 0x1F;

	faces = (blockData.x >> 15) & 0x3F;

	ivec3 worldOffset = chunks[gl_DrawID].worldpos.zyx * 32;

	gl_Position = vec4(worldOffset + ivec3(position), 1.0);
}
