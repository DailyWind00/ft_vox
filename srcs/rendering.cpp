#include "config.hpp"

// Keep the window alive, exiting this function should mean closing the window
static void program_loop(Window &window, Profiler &pr, Shader &shader) {
	printVerbose("Entering program's loop\n");

	// To remove
	VoxelSystem<uint8_t, 32>	voxelSystem;
	// --

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Update
		static float angle = 0.0f;
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			angle -= 0.01f;

		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			angle += 0.01f;

		glm::mat4 model      = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around Y axis
		glm::mat4 view       = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
		glm::mat4 mvp        = projection * view * model;
		shader.setUniform("transform", mvp);

		voxelSystem.draw();

		handleEvents(window);
		glfwSwapBuffers(window);
	}

	(void)pr;
	printVerbose("Exiting program's loop\n");
}

// Setup variables and call the program loop
void	Rendering(Window &window) {
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Profiler	pr;
	Shader		shader("shaders/vertex.glsl", "shaders/fragment.glsl", "shaders/geometry.glsl");
	shader.use();

	program_loop(window, pr, shader);

	pr.logToFile("out");
}
