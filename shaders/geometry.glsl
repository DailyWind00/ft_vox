#version 460 core

layout (points) in;
layout (triangle_strip, max_vertices = 14) out; // 14 vertices for a cube without redundancies

uniform mat4	transform;
uniform vec3	camPos;

flat in uint faces[]; // xxyyzz

out vec3	fragPos;

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
void main() {
	vec4 position = gl_in[0].gl_Position;
	fragPos = position.xyz;
	
	// TODO: Fix visible faces (bitmask)

	// x axis faces
	if (camPos.x > position.x && bool(faces[0] & 0x1))
		makeRightFace(position);
	if (camPos.x < position.x && bool(faces[0] & 0x2))
		makeLeftFace(position);

	// y axis faces
	if (camPos.y > position.y && bool(faces[0] & 0x4))
		makeTopFace(position);
	if (camPos.y < position.y && bool(faces[0] & 0x8))
		makeBottomFace(position);
	
	// z axis faces
	if (camPos.z > position.z && bool(faces[0] & 0x16))
		makeFrontFace(position);
	if (camPos.z < position.z && bool(faces[0] & 0x32))
		makeBackFace(position);
	
}
