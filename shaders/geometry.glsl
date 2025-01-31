#version 460 core

layout (points) in;
layout (triangle_strip, max_vertices = 12) out; // 4 vertex * 3 faces always visibles

uniform mat4	transform;
uniform vec3	camPos;

flat in uint faces[1]; // xxyyzz

out vec3	fragPos;
out float 	fragBlockColor;

// Offsets for the 8 corners of the cube
vec4 offsets[8] = vec4[](
	vec4(0, 0, 0, 0.0), // 0: Bottom-left-back
    vec4(1, 0, 0, 0.0), // 1: Bottom-right-back
    vec4(0, 1, 0, 0.0), // 2: Top-left-back
    vec4(1, 1, 0, 0.0), // 3: Top-right-back
    vec4(0, 0, 1, 0.0), // 4: Bottom-left-front
    vec4(1, 0, 1, 0.0), // 5: Bottom-right-front
    vec4(0, 1, 1, 0.0), // 6: Top-left-front
    vec4(1, 1, 1, 0.0)  // 7: Top-right-front
);

void	makeFrontFace(vec4 position) {
	gl_Position = transform * vec4(position + offsets[4]); EmitVertex();
	gl_Position = transform * vec4(position + offsets[5]); EmitVertex();
	gl_Position = transform * vec4(position + offsets[6]); EmitVertex();
	gl_Position = transform * vec4(position + offsets[7]); EmitVertex();
	EndPrimitive();
}

void	makeTopFace(vec4 position) {
	gl_Position = transform * vec4(position + offsets[6]); EmitVertex();
	gl_Position = transform * vec4(position + offsets[7]); EmitVertex();
	gl_Position = transform * vec4(position + offsets[2]); EmitVertex();
	gl_Position = transform * vec4(position + offsets[3]); EmitVertex();
	EndPrimitive();
}

void	makeBackFace(vec4 position) {
	gl_Position = transform * vec4(position + offsets[0]); EmitVertex();
	gl_Position = transform * vec4(position + offsets[1]); EmitVertex();
	gl_Position = transform * vec4(position + offsets[2]); EmitVertex();
	gl_Position = transform * vec4(position + offsets[3]); EmitVertex();
	EndPrimitive();
}

void	makeBottomFace(vec4 position) {
	gl_Position = transform * vec4(position + offsets[0]); EmitVertex();
	gl_Position = transform * vec4(position + offsets[1]); EmitVertex();
	gl_Position = transform * vec4(position + offsets[4]); EmitVertex();
	gl_Position = transform * vec4(position + offsets[5]); EmitVertex();
	EndPrimitive();
}

void	makeLeftFace(vec4 position) {
	gl_Position = transform * vec4(position + offsets[0]); EmitVertex();
	gl_Position = transform * vec4(position + offsets[4]); EmitVertex();
	gl_Position = transform * vec4(position + offsets[2]); EmitVertex();
	gl_Position = transform * vec4(position + offsets[6]); EmitVertex();
	EndPrimitive();
}

void	makeRightFace(vec4 position) {
	gl_Position = transform * vec4(position + offsets[1]); EmitVertex();
	gl_Position = transform * vec4(position + offsets[5]); EmitVertex();
	gl_Position = transform * vec4(position + offsets[3]); EmitVertex();
	gl_Position = transform * vec4(position + offsets[7]); EmitVertex();
	EndPrimitive();
}

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
	vec4 position = gl_in[0].gl_Position;
	fragPos = position.xyz;
	fragBlockColor = 0.9 * rand(fragPos.xz);
	
	// TODO: Fix visible faces (bitmask)

	// x axis faces
	if (camPos.x < position.x && bool(faces[0] & 0x16))
		makeRightFace(position);
	if (camPos.x > position.x && bool(faces[0] & 0x32))
		makeLeftFace(position);

	// y axis faces
	if (camPos.y < position.y && bool(faces[0] & 0x4))
		makeTopFace(position);
	if (camPos.y > position.y && bool(faces[0] & 0x8))
		makeBottomFace(position);
	
	// z axis faces
	if (camPos.z < position.z && bool(faces[0] & 0x1))
		makeFrontFace(position);
	if (camPos.z > position.z && bool(faces[0] & 0x2))
		makeBackFace(position);
}
