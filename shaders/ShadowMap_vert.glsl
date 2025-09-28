#version 460 core

layout (location = 0) in uvec2 vertexData;

uniform mat4	view;
uniform mat4	projection;
uniform vec3	worldPos;

uvec3	decodePosition() {
	uvec3	pos = uvec3(0);
	pos.x = (vertexData[0])        & 0x3F;
	pos.y = (vertexData[0] >> 6)   & 0x3F;
	pos.z = (vertexData[0] >> 12)  & 0x3F;
	return pos;
}

void	main() {
	// Decode Vertex Data
	uvec3	pos = decodePosition();

	gl_Position = projection * view * vec4(ivec3(pos) + ivec3(32 * worldPos), 1.0f);
}
