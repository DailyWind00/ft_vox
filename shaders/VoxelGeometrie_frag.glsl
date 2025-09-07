#version 460 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gColor;

in vec3	fragPos;
in vec3	Normal;
in vec2	uv;
in vec2	l;
flat in uint	texID;
flat in uint	face;

uniform float		time;
uniform sampler2D	atlas;

void	main()
{
	gPosition = vec4(fragPos, 1.0f);
	gNormal = vec4(Normal, 1.0f);

	uint	xOff = texID % 16;
	uint	yOff = texID / 16;

	gColor = texture(atlas, vec2((fract(uv.x * l.x) + xOff) / 16, (fract(uv.y * l.y) + yOff) / 16));
	// gColor = vec4(vec3(float(face) / 6.0f), 1.0f);
}
