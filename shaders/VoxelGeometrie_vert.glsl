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

const int	spriteNum = 3;
const uint	spriteIDS[spriteNum] = {
	5, 6, 7
};

mat3	rotate3d(float _angle)
{
	return mat3(cos(_angle), 0.0f, sin(_angle),
		    0.0f, 1.0f, 0.0f,
		    -sin(_angle), 0.0f, cos(_angle));
}

uint	getBlockType(const uint id)
{
	for (int i = 0; i < spriteNum; i++) {
		if (id == spriteIDS[i])
			return (1);
	}
	return (0);
}

vec3	contructBlock(const vec2 len)
{
	vec3	finalQuad = vec3(0);

	// x axis
	if (face == 1) {
		finalQuad = quad.yzx;
		finalQuad.z *= len.x;
		finalQuad.y *= len.y;

		finalQuad.y -= len.y - 1;
		finalQuad.x += len.y - 1;
		uv = UVs;
	}
	else if (face == 0) {
		finalQuad = quad.yxz;
		finalQuad.z *= len.x;
		finalQuad.y *= len.y;

		finalQuad.y -= len.y - 1;
		finalQuad.x = 0;
		uv = UVs.yx;
	}
	
	// y axis
	if (face == 3) {
		finalQuad = quad;
		finalQuad.z *= len.x;
		finalQuad.x *= len.y;
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
		finalQuad.y -= len.y - 1;
		finalQuad.z += len.y - 1;
		uv = UVs.yx;
	}
	else if (face == 4) {
		finalQuad = quad.xzy;
		finalQuad.x *= len.x;
		finalQuad.y *= len.y;
		finalQuad.y -= len.y - 1;
		finalQuad.z += len.y - 1;
		finalQuad.z = 0;
		uv = UVs;
	}
	return (finalQuad);
}

vec3	contructSprite(const vec2 len)
{
	vec3	finalQuad = vec3(0);
	
	// x axis
	if (face == 1) {
		finalQuad = quad.yzx;
		finalQuad = vec3(rotate3d(0.785398) * finalQuad.xyz);
		finalQuad.x += 0.06;
		finalQuad.z -= 0.56;
		uv = UVs;
	}
	else if (face == 0) {
		finalQuad = quad.yxz;
		finalQuad = vec3(rotate3d(0.785398) * finalQuad.xyz);
		finalQuad.x += 0.06;
		finalQuad.z -= 0.56;
		uv = UVs.yx;
	}

	// z axis
	if (face == 5) {
		finalQuad = quad.zxy;
		finalQuad = vec3(rotate3d(0.785398) * finalQuad.xyz);
		finalQuad.z -= 0.56;
		finalQuad.x += 0.76;
		uv = UVs.yx;
	}
	else if (face == 4) {
		finalQuad = quad.xzy;
		finalQuad = vec3(rotate3d(0.785398) * finalQuad.xyz);
		finalQuad.z -= 0.56;
		finalQuad.x += 0.76;
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
	face = meshData[gl_DrawID].data.w;

	len.x = ((blockData >> 22) & 0x1F) + 1;
	len.y = ((blockData >> 27) & 0x1F) + 1;

	// Fragment shaders datas
	fragPos = vec4(view * vec4(ivec3(pos) + worldOffset, 1.0f)).xyz;
	Normal = Normals[meshData[gl_DrawID].data.w];
	l = len;

	// Get the block type
	uint	blockType = getBlockType(texID);

	// Create the final face mesh
	vec3	fQuad;
	
	if (blockType == 0)
		fQuad = contructBlock(len);
	else if (blockType == 1)
		fQuad = contructSprite(len);

	gl_Position = projection * view * vec4((fQuad + ivec3(pos) + worldOffset), 1.0f);
}
