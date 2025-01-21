#version 420 core

layout (location = 0) in uint blockData;

uniform mat4 transform; // Projection * View * Model

void	main() {
	gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
}