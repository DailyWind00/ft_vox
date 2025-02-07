#version 460 core

out vec4	ScreenColor;

in float	randFactor;
in vec3		fragPos;
flat in uint	face;

void	main()
{
	//-ScreenColor= vec4((face * 1.8) * (0.2 * vec3(randFactor)) + ivec3(fragPos) * 0.01, 1.0);
	ScreenColor = vec4(vec3(0.3 * (face + 1) * (fragPos.y + 100) * 0.005), 1.0f);
}
