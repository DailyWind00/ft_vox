#version 460 core

out vec4 ScreenColor;

in vec3	fragPos;

void	main() {
	ScreenColor = vec4(ivec3(fragPos) * 0.04, 1.0);
}
