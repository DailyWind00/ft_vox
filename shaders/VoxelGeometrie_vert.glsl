#version 460 core

layout (location = 0) in uvec2 vertexData;

uniform mat4	view;
uniform mat4	projection;
uniform vec3	worldPos;
uniform vec3	camPos;

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

uvec3	decodePosition() {
	uvec3	pos = uvec3(0);
	pos.x = (vertexData[0])        & 0x3F;
	pos.y = (vertexData[0] >> 6)   & 0x3F;
	pos.z = (vertexData[0] >> 12)  & 0x3F;
	return pos;
}

void	decodeUVs() {
	uv.x = (vertexData[0] >> 27) & 0x01;
	uv.y = (vertexData[0] >> 28) & 0x01;
}

void	decodeLengths() {
	l.x = (vertexData[1]) & 0x3F;
	l.y = (vertexData[1] >> 6) & 0x3F;
}

void	main() {
	// Decode Vertex Data
	uvec3	pos = decodePosition();
	vec3	fPos = pos;

	decodeUVs();
	decodeLengths();
	face = (vertexData[0] >> 18) & 0x07;
	texID = ((vertexData[0] >> 22) & 0x1F) - 1;

	if (texID == 8)
		fPos.y -= 2.0f / 16.0f;

	// Set rendering data
	Normal = Normals[face];
	fragPos = (vec4(fPos + vec3(32 * worldPos), 1.0f)).xyz;

	gl_Position = projection * view * vec4(fPos + vec3(32 * worldPos), 1.0f);
}
