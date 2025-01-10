#include "config.hpp"

// Keep the window alive, exiting this function should mean closing the window
static void program_loop(Window &window, Profiler &pr, Shader &shader) {
	// To remove
	glm::vec3	positions[] = {
		glm::vec3(0.0f, 0.0f, 0.0f),
	};

	GLuint VAO, VBO;	
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);
	// --

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// Update
		glm::mat4 model      = glm::mat4(1.0f); // Identity matrix
		glm::mat4 view       = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
		glm::mat4 mvp        = projection * view * model;
		shader.setUniform("transform", mvp);

		// Draw call
		glDrawArrays(GL_POINTS, 0, 1);

		handleEvents(window);
		glfwSwapBuffers(window);
	}

	(void)pr;
	(void)shader;
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

	printVerbose("Entering program's loop");
	program_loop(window, pr, shader);
	printVerbose("Exiting program's loop");

	pr.logToFile("out");
}