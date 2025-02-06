#version 460 core

struct Chunk {
	ivec4	data; // Fuck you khronos
};

layout (location = 0) in vec3 quad;
layout (location = 1) in uint blockData;
layout (std430, binding = 0) buffer SSBO {
	Chunk meshData[];
};

uniform mat4	transform;

out vec3	fragPos;
out float	randFactor;

float rand(vec2 co)
{
	return (fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453));
}

void main()
{
	// Decode blockData bitmask :
	uvec3	position = uvec3(0);

	position.x = (blockData)       & 0x1F;
	position.y = (blockData >> 5)  & 0x1F;
	position.z = (blockData >> 10) & 0x1F;

	uint face = (blockData >> 15) & 0x3F;

	ivec3 worldOffset = meshData[gl_DrawID].data.zyx * 32;
	fragPos = vec3(ivec3(position) + worldOffset);

	randFactor = rand(fragPos.xz) * rand(fragPos.yz) * rand(fragPos.xy);

	gl_Position = transform * vec4((quad + ivec3(position) + worldOffset), 1.0f);
}
