# version 420 core

layout (location = 0) in vec3 quad;
layout (location = 1) in vec2 UV;

out vec2	uv;

void	main()
{
	uv = UV;
	
	gl_Position = vec4(quad, 1.0f);
}
