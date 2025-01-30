#version 460 core

layout (points) in;
layout (triangle_strip, max_vertices = 14) out; // 14 vertices for a cube without redundancies

void main() {
	vec4 position = gl_in[0].gl_Position;

	// Offsets for the 8 corners of the cube
	vec4 offsets[8] = vec4[](
		vec4(0.0, 0.0, 0.0, 0.0), // 0: Bottom-left-back
		vec4(1.0, 0.0, 0.0, 0.0), // 1: Bottom-right-back
		vec4(0.0, 1.0, 0.0, 0.0), // 2: Top-left-back
		vec4(1.0, 1.0, 0.0, 0.0), // 3: Top-right-back
		vec4(0.0, 0.0, 1.0, 0.0), // 4: Bottom-left-front
		vec4(1.0, 0.0, 1.0, 0.0), // 5: Bottom-right-front
		vec4(0.0, 1.0, 1.0, 0.0), // 6: Top-left-front
		vec4(1.0, 1.0, 1.0, 0.0)  // 7: Top-right-front
	);

	// Emit vertices in a single triangle strip
	gl_Position = position + offsets[0]; EmitVertex(); // Bottom-left-back
	gl_Position = position + offsets[1]; EmitVertex(); // Bottom-right-back
	gl_Position = position + offsets[2]; EmitVertex(); // Top-left-back
	gl_Position = position + offsets[3]; EmitVertex(); // Top-right-back
	gl_Position = position + offsets[7]; EmitVertex(); // Top-right-front
	gl_Position = position + offsets[5]; EmitVertex(); // Bottom-right-front
	gl_Position = position + offsets[4]; EmitVertex(); // Bottom-left-front
	gl_Position = position + offsets[6]; EmitVertex(); // Top-left-front
	gl_Position = position + offsets[2]; EmitVertex(); // Top-left-back
	gl_Position = position + offsets[7]; EmitVertex(); // Top-right-front
	gl_Position = position + offsets[3]; EmitVertex(); // Top-right-back
	gl_Position = position + offsets[1]; EmitVertex(); // Bottom-right-back
	gl_Position = position + offsets[0]; EmitVertex(); // Bottom-left-back
	gl_Position = position + offsets[4]; EmitVertex(); // Bottom-left-front
	EndPrimitive();
}