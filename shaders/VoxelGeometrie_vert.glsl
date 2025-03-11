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

uniform mat4	transform;
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

float rand(vec2 co) {
	return (fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453));
}

void main() {
	ivec3 worldOffset = meshData[gl_DrawID].data.zyx * 32;

	// Decode blockData bitmask :
	uvec3	pos = uvec3(0);
	uvec2	len = uvec2(0);

	pos.x = (blockData)        & 0x1F;
	pos.y = (blockData >> 5)   & 0x1F;
	pos.z = (blockData >> 10)  & 0x1F;

	texID = ((blockData >> 15) & 0x7F);

	len.x = ((blockData >> 22) & 0x1F) + 1;
	len.y = ((blockData >> 27) & 0x1F) + 1;

	// Color modifiers for the fragment shader
	fragPos = vec3(ivec3(pos) + worldOffset);

	// Create the final face mesh
	vec3	fQuad;

	face = meshData[gl_DrawID].data.w;
	
	// x axis
	if (face == 1) {
		fQuad = quad.yzx;
		fQuad.z *= len.x;
		fQuad.y *= len.y;

		fQuad.y -= len.y - 1;
		fQuad.x += len.y - 1;
		uv = UVs;
	}
	else if (face == 0) {
		fQuad = quad.yxz;
		fQuad.z *= len.x;
		fQuad.y *= len.y;

		fQuad.y -= len.y - 1;
		fQuad.x = 0;
		uv = UVs.yx;
	}
	
	// y axis
	if (face == 3) {
		fQuad = quad;
		fQuad.z *= len.x;
		fQuad.x *= len.y;
		uv = UVs.yx;
	}
	else if (face == 2) {
		fQuad = quad.zyx;
		fQuad.z *= len.x;
		fQuad.x *= len.y;
		fQuad.y = 0;
		uv = UVs;
	}

	// z axis
	if (face == 5) {
		fQuad = quad.zxy;
		fQuad.x *= len.x;
		fQuad.y *= len.y;
		fQuad.y -= len.y - 1;
		fQuad.z += len.y - 1;
		uv = UVs.yx;
	}
	else if (face == 4) {
		fQuad = quad.xzy;
		fQuad.x *= len.x;
		fQuad.y *= len.y;
		fQuad.y -= len.y - 1;
		fQuad.z += len.y - 1;
		fQuad.z = 0;
		uv = UVs;
	}

	Normal = Normals[meshData[gl_DrawID].data.w];

	l = len;

	gl_Position = transform * vec4((fQuad + ivec3(pos) + worldOffset), 1.0f);
}
