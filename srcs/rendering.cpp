#include "config.hpp"

// Keep the window alive, exiting this function should mean closing the window
static void program_loop(Window &window, VoxelSystem<uint8_t, 32> &voxelSystem, Shader &shader) {
	printVerbose("Entering program's loop\n");

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		voxelSystem.draw();

		handleEvents(window, shader);
		glfwSwapBuffers(window);
	}

	printVerbose("Exiting program's loop\n");
}

// Setup variables and call the program loop
void	Rendering(Window &window) {
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	VoxelSystem<uint8_t, 32>	voxelSystem;
	Shader	shader(
		"shaders/vertex.glsl",
		"shaders/fragment.glsl",
		"shaders/geometry.glsl"
	);
	shader.use();

	program_loop(window, voxelSystem, shader);
}
