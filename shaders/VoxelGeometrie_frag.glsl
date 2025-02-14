#version 460 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gColor;

in vec3		fragPos;
in vec3		Normal;

void	main()
{
	gPosition = vec4(fragPos, 1.0f);
	gNormal = vec4(Normal, 1.0f);
	gColor = vec4(0.4, 0.75, 0.3, 1.0f);
}
