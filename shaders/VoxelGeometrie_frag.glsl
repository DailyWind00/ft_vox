#version 420 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gColor;

in vec3	fragPos;
in vec3	Normal;
in vec2	uv;
in vec2	l;
flat in uint	texID;
flat in uint	face;

uniform bool		polygonVisible;
uniform sampler2D	atlas;

float	sdfSegment(vec2 p, vec2 a, vec2 b) {
	float	h = min(1.0, max(0.0, dot(p - a, b - a) / dot(b - a, b - a)));
	return length(p - (a + h * (b - a)));
}

float	addSegment(float d1, float d2, float d3, float d4, float d5) {
	return min(min(min(min(d1, d2), d3), d4), d5);
}

void	main()
{
	gPosition = vec4(fragPos, 1.0f);
	gNormal = vec4(Normal, 1.0f);

	uint	xOff = texID % 16;
	uint	yOff = texID / 16;

	float	thickness = 0.01 / ((l.x + l.y) / 2.0f);
	float	roughness = 0.01 / ((l.x + l.y) / 2.0f);

	vec3	color = texture(atlas, vec2((fract(uv.x * l.x) + xOff) / 16, (fract(uv.y * l.y) + yOff) / 16)).rgb;

	if (polygonVisible) {
		float	d = addSegment(sdfSegment(uv, vec2(0, 0), vec2(1, 0)),
			sdfSegment(uv, vec2(0, 0), vec2(0, 1)),
			sdfSegment(uv, vec2(1, 1), vec2(1, 0)),
			sdfSegment(uv, vec2(1, 1), vec2(0, 1)),
			sdfSegment(uv, vec2(0, 0), vec2(1, 1)));
		float	sm = 1.0f - smoothstep(thickness, (thickness + roughness), d);
		vec3	polygonColor = (1.0f - color) * sm;
		color *= 1.0f - sm;
		color += polygonColor;
	}

	gColor = vec4(color, 1.0f);
}
