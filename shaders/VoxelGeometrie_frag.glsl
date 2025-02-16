#version 460 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gColor;

in vec3	fragPos;
in vec3	Normal;
in vec2	uv;
in float	l;
in float	texOffset;

uniform float		time;
uniform sampler2D	atlas;

void	main()
{
	gPosition = vec4(fragPos, 1.0f);
	gNormal = vec4(Normal, 1.0f);
	gColor = texture(atlas, vec2((fract(uv.x * l) + texOffset) / 16, uv.y / 16));
	
}
