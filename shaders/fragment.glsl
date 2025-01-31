#version 460 core

out vec4 ScreenColor;

in vec3	fragPos;
in float fragBlockColor;

void	main() {
	ScreenColor = vec4(fragBlockColor + ivec3(fragPos) * 0.01, 1.0);
	// ScreenColor = vec4(0.9 * vec3(rand(fragPos.xz)) + ivec3(fragPos) * 0.01, 1.0);
}
