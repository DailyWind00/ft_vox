#version 460 core

out vec4 ScreenColor;

in vec3	fragPos;

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}
void	main() {
	ScreenColor = vec4(0.9 * vec3(rand(fragPos.xz)) + ivec3(fragPos) * 0.01, 1.0);
}
