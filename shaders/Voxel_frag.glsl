#version 460 core

out vec4	ScreenColor;

in float	randFactor;
in vec3		fragPos;
flat in uint	face;
in vec3		Normal;

uniform vec3	sunPos;
uniform float	time;

void	main()
{
	vec3	ambColor = vec3(0.3, 0.75, 0.3) * 0.1;

	vec3	lDir = sunPos;

	float	diffuse = max(dot(Normal, lDir), 0.0);
	vec3	diffColor = diffuse * vec3(0.4, 0.75, 0.2);

	//-ScreenColor= vec4((face * 1.8) * (0.2 * vec3(randFactor)) + ivec3(fragPos) * 0.01, 1.0);
	//-ScreenColor = vec4(vec3(0.3 * (face + 1) * (fragPos.y + 100) * 0.005), 1.0f);
	ScreenColor = vec4(diffColor + ambColor, 1.0f);
}
