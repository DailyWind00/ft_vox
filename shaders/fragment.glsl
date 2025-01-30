#version 460 core

in flat int fragLoad; // bool

out vec4 ScreenColor;

void	main() {
	if (!bool(fragLoad))
		discard;

	ScreenColor = vec4(1.0, 1.0, 1.0, 1.0);
}