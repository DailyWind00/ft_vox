#version 460 core

// struct Chunk {
// 	ivec4	data; // Fuck you khronos
// };

layout (location = 0) in uvec2 quad;
// layout (location = 1) in uint blockData;
// layout (location = 2) in vec2 UVs;
// layout (std430, binding = 0) buffer SSBO {
// 	Chunk meshData[];
// };

uniform mat4	view;
uniform mat4	projection;
uniform float	time;
uniform vec3	worldPos;

out vec2	uv;
out vec3	Normal;
out vec3	fragPos;
out vec2	l;
flat out uint	texID;
flat out uint	face;

const vec3	Normals[] = {
	vec3( 0, 0,-1),
	vec3( 0, 0, 1),
	vec3( 0,-1, 0),
	vec3( 0, 1, 0),
	vec3(-1, 0, 0),
	vec3( 1, 0, 0)
};

void main() {
	uvec3	pos = uvec3(0);
	pos.x = (quad[0])        & 0x3F;
	pos.y = (quad[0] >> 6)   & 0x3F;
	pos.z = (quad[0] >> 12)  & 0x3F;

	uv.x = (quad[0] >> 27) & 0x01;
	uv.y = (quad[0] >> 28) & 0x01;

	face = (quad[0] >> 18) & 0x07;
	Normal = Normals[face];
	fragPos = (view * vec4(ivec3(pos) + ivec3(32 * worldPos), 1.0f)).xyz;
	texID = ((quad[0] >> 22) & 0x1F) - 1;

	l.x = (quad[1]) & 0x3F;
	l.y = (quad[1] >> 6) & 0x3F;

	gl_Position = projection * view * vec4(ivec3(pos) + ivec3(32 * worldPos), 1.0f);
}
