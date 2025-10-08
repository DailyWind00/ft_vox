# version 420 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gColor;

in vec4	FragPos;
in vec4 Normal;
in vec2	uv;

uniform int		texID;
uniform sampler2D	atlas;

void main() {
	gPosition = FragPos;
	gNormal = Normal;

	uvec2	off = uvec2(texID % 16, texID / 16);

	gColor = texture(atlas, (uv + off) / 16);
}
