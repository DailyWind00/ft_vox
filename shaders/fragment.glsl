#version 460 core

out vec4	ScreenColor;

in float	randFactor;
in vec3		fragPos;

void	main()
{
	ScreenColor= vec4(1.5 * vec3(randFactor) + ivec3(fragPos) * 0.01, 1.0);
}
