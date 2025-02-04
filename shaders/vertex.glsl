#version 460 core

struct Chunk {
	ivec4	data; // Fuck you khronos
};

layout (location = 0) in uint blockData;
layout (std430, binding = 0) buffer SSBO {
	Chunk meshData[];
};

flat out ivec3	geoPos;

void main() {
	// Decode blockData bitmask :
	uvec3	position;
	position.x = (blockData)       & 0x1F;
	position.y = (blockData >> 5)  & 0x1F;
	position.z = (blockData >> 10) & 0x1F;

	ivec3 worldOffset = meshData[gl_DrawID].data.zyx * 32;

	geoPos = ivec3(position);
	gl_Position = vec4(worldOffset + geoPos , 1.0);
}
