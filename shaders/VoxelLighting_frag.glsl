# version 460 core

out vec4	ScreenColor;
in vec2		uv;

uniform sampler2D	gPosition;
uniform sampler2D	gNormal;
uniform sampler2D	gColor;

uniform vec3	sunPos;

void	main()
{
	vec4	fColor = vec4(0);

	vec4	fragPos = texture(gPosition, uv);
	vec4	Normal = texture(gNormal, uv);
	vec4	Color = texture(gColor, uv);

	vec3	ambColor = Color.rgb * 0.3;
	vec3	lDir = sunPos;

	float	diffuse = max(dot(Normal.rgb, lDir), 0.0);
	vec3	diffColor = diffuse * Color.rgb;

	//-ScreenColor= vec4((face * 1.8) * (0.2 * vec3(randFactor)) + ivec3(fragPos) * 0.01, 1.0);
	//-ScreenColor = vec4(vec3(0.3 * (face + 1) * (fragPos.y + 100) * 0.005), 1.0f);
	//-ScreenColor = vec4(diffColor + ambColor, 1.0f);
	ScreenColor = vec4(diffColor + ambColor, 1.0f);
}
