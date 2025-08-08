#version 460 core

struct Chunk {
	ivec4	data; // Fuck you khronos
};

layout (location = 0) in vec3 quad;
layout (location = 1) in uint blockData;
layout (location = 2) in vec2 UVs;
layout (std430, binding = 0) buffer SSBO {
	Chunk meshData[];
};

uniform mat4	view;
uniform mat4	projection;
uniform float	time;

out vec2	uv;
out vec3	Normal;
out vec3	fragPos;
out vec2	l;
flat out uint	texID;
flat out uint	face;

const vec3	Normals[] = {
	vec3(-1, 0, 0),
	vec3( 1, 0, 0),
	vec3( 0,-1, 0),
	vec3( 0, 1, 0),
	vec3( 0, 0,-1),
	vec3( 0, 0, 1)
};

vec3	contructBlock(const vec2 len, const uint LOD)
{
	vec3	finalQuad = vec3(0);

	// x axis
	if (face == 1) {
		finalQuad = quad.yzx;
		finalQuad.z *= len.x;
		finalQuad.y *= len.y;

		finalQuad.x += LOD - 1;
		uv = UVs;
	}
	else if (face == 0) {
		finalQuad = quad.yxz;
		finalQuad.z *= len.x;
		finalQuad.y *= len.y;
		finalQuad.x = 0;
		uv = UVs.yx;
	}
	
	// y axis
	if (face == 3) {
		finalQuad = quad;
		finalQuad.z *= len.x;
		finalQuad.x *= len.y;
		finalQuad.y += LOD - 1;
		uv = UVs.yx;
	}
	else if (face == 2) {
		finalQuad = quad.zyx;
		finalQuad.z *= len.x;
		finalQuad.x *= len.y;
		finalQuad.y = 0;
		uv = UVs;
	}

	// z axis
	if (face == 5) {
		finalQuad = quad.zxy;
		finalQuad.x *= len.x;
		finalQuad.y *= len.y;
		finalQuad.z += LOD - 1;
		uv = UVs.yx;
	}
	else if (face == 4) {
		finalQuad = quad.xzy;
		finalQuad.x *= len.x;
		finalQuad.y *= len.y;
		finalQuad.z = 0;
		uv = UVs;
	}
	return (finalQuad);
}

void main() {
	ivec3 worldOffset = meshData[gl_DrawID].data.zyx * 32;

	// Decode blockData bitmask :
	uvec3	pos = uvec3(0);
	uvec2	len = uvec2(0);

	pos.x = (blockData)        & 0x1F;
	pos.y = (blockData >> 5)   & 0x1F;
	pos.z = (blockData >> 10)  & 0x1F;

	texID = ((blockData >> 15) & 0x7F) - 1;
	face = meshData[gl_DrawID].data.w & 0x0F;
	uint	LOD = (meshData[gl_DrawID].data.w >> 4) & 0xFF;

	len.x = ((blockData >> 22) & 0x1F) + 1;
	len.y = ((blockData >> 27) & 0x1F) + 1;

	// Fragment shaders datas
	fragPos = vec4(view * vec4(ivec3(pos) + worldOffset, 1.0f)).xyz;
	Normal = Normals[face];
	l = len;

	// Create the final face mesh
	vec3	fQuad = contructBlock(len, LOD);
	fQuad.y -= LOD - 1;

	gl_Position = projection * view * vec4((fQuad + ivec3(pos) + worldOffset), 1.0f);
}
