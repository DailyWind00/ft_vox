#version 420 core

layout (location = 0) in ivec3 blockData;

uniform mat4 transform; // Projection * View * Model

void	main() {
	gl_Position = transform * vec4(blockData, 1.0);
}