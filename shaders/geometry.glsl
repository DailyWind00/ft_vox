#version 460 core

layout (points) in;
layout (triangle_strip, max_vertices = 14) out; // 14 vertices for a cube without redundancies

uniform mat4	transform;
uniform vec3	camPos;

flat in ivec3	geoPos[];
out vec3	fragPos;

// Offsets for the 8 corners of the cube
vec4 offsets[8] = vec4[](
	vec4(-0.5, -0.5, -0.5, 0.0), // 0: Bottom-left-back
	vec4( 0.5, -0.5, -0.5, 0.0), // 1: Bottom-right-back
	vec4(-0.5,  0.5, -0.5, 0.0), // 2: Top-left-back
	vec4( 0.5,  0.5, -0.5, 0.0), // 3: Top-right-back
	vec4(-0.5, -0.5,  0.5, 0.0), // 4: Bottom-left-front
	vec4( 0.5, -0.5,  0.5, 0.0), // 5: Bottom-right-front
	vec4(-0.5,  0.5,  0.5, 0.0), // 6: Top-left-front
	vec4( 0.5,  0.5,  0.5, 0.0)  // 7: Top-right-front
);

void	makeFrontFace(vec4 position) {
	gl_Position = transform * vec4(position + offsets[4]); EmitVertex(); // Bottom-left-back
	gl_Position = transform * vec4(position + offsets[5]); EmitVertex(); // Bottom-left-back
	gl_Position = transform * vec4(position + offsets[6]); EmitVertex(); // Bottom-left-back
	gl_Position = transform * vec4(position + offsets[7]); EmitVertex(); // Bottom-left-back
	EndPrimitive();
}

void	makeTopFace(vec4 position) {
	gl_Position = transform * vec4(position + offsets[6]); EmitVertex(); // Bottom-left-back
	gl_Position = transform * vec4(position + offsets[7]); EmitVertex(); // Bottom-left-back
	gl_Position = transform * vec4(position + offsets[2]); EmitVertex(); // Bottom-left-back
	gl_Position = transform * vec4(position + offsets[3]); EmitVertex(); // Bottom-left-back
	EndPrimitive();
}

void	makeBackFace(vec4 position) {
	gl_Position = transform * vec4(position + offsets[0]); EmitVertex(); // Bottom-left-back
	gl_Position = transform * vec4(position + offsets[1]); EmitVertex(); // Bottom-left-back
	gl_Position = transform * vec4(position + offsets[2]); EmitVertex(); // Bottom-left-back
	gl_Position = transform * vec4(position + offsets[3]); EmitVertex(); // Bottom-left-back
	EndPrimitive();
}

void	makeBottomFace(vec4 position) {
	gl_Position = transform * vec4(position + offsets[0]); EmitVertex(); // Bottom-left-back
	gl_Position = transform * vec4(position + offsets[1]); EmitVertex(); // Bottom-left-back
	gl_Position = transform * vec4(position + offsets[4]); EmitVertex(); // Bottom-left-back
	gl_Position = transform * vec4(position + offsets[5]); EmitVertex(); // Bottom-left-back
	EndPrimitive();
}

void	makeLeftFace(vec4 position) {
	gl_Position = transform * vec4(position + offsets[0]); EmitVertex(); // Bottom-left-back
	gl_Position = transform * vec4(position + offsets[4]); EmitVertex(); // Bottom-left-back
	gl_Position = transform * vec4(position + offsets[2]); EmitVertex(); // Bottom-left-back
	gl_Position = transform * vec4(position + offsets[6]); EmitVertex(); // Bottom-left-back
	EndPrimitive();
}

void	makeRightFace(vec4 position) {
	gl_Position = transform * vec4(position + offsets[1]); EmitVertex(); // Bottom-left-back
	gl_Position = transform * vec4(position + offsets[5]); EmitVertex(); // Bottom-left-back
	gl_Position = transform * vec4(position + offsets[3]); EmitVertex(); // Bottom-left-back
	gl_Position = transform * vec4(position + offsets[7]); EmitVertex(); // Bottom-left-back
	EndPrimitive();
}
void main() {
	vec4 position = gl_in[0].gl_Position;
	fragPos = position.xyz;
	
	if (camPos.x > position.x)
		makeRightFace(position);
	else
		makeLeftFace(position);

	if (camPos.y > position.y)
		makeTopFace(position);
	else
		makeBottomFace(position);
	
	if (camPos.z > position.z)
		makeFrontFace(position);
	else
		makeBackFace(position);
	
}
