#version 420 core

layout (location = 0) in vec3 position;

uniform mat4 transform; // Projection * View * Model

void	main() {
	gl_Position = vec4(position, 1.0);
}