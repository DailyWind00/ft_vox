#version 460 core

out vec4 ScreenColor;

in vec3	fragPos;
in float fragBlockColor;
flat in int	id;

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}
void	main() {
	float	randFactor = rand(fragPos.xz) * rand(fragPos.yz) * rand(fragPos.xy);

	ScreenColor= vec4(1.5 * vec3(randFactor) + ivec3(fragPos) * 0.01, 1.0);
}
