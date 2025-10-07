# version 420 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gColor;

in vec4	FragPos;
in vec4 Normal;

void	main() {
	gColor = vec4(1.0, 0.0, 0.0, 1.0);
}
