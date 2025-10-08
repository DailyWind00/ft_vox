# version 420 core

layout (location = 0) in vec3	pos;
layout (location = 1) in vec2	UV;
layout (location = 2) in int	face;

uniform mat4	projection;

out vec4	FragPos;
out vec4	Normal;
out vec2	uv;

const vec3	Normals[] = {
	{0, 0, 1},
	{0, 1, 0},
	{-1, 0, 0}
};

void	main() {
	vec3	position = pos + vec3(0.75f, -0.35f, -0.75f);

	uv = UV;
	FragPos = vec4(position, 1.0f);
	Normal = vec4(Normals[face], 1.0f);
	gl_Position = projection * vec4(position, 1.0f);
}
