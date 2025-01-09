#include "config.hpp"

// Temporary function to test the profiler (to remove)
static void	countTo(int num)
{
	int	i = 0;

	while (i < num)
		i++;
}

// Keep the window alive, exiting this function should mean closing the window
static void program_loop(Window &window, Profiler &pr) {
	while (!glfwWindowShouldClose(window)) {
		glfwSwapBuffers(window);

		// temporary (to remove)
		pr.evaluateNoReturn("countTo100000000", &countTo, 100000000);
		pr.evaluateNoReturn("countTo10000000", &countTo, 10000000);
		pr.evaluateNoReturn("countTo1000000", &countTo, 1000000);
		pr.evaluateNoReturn("countTo100000", &countTo, 100000);
		pr.evaluateNoReturn("countTo10000", &countTo, 10000);
		pr.evaluateNoReturn("countTo1000", &countTo, 1000);
		// --

		handleEvents(window);
	}
}

// Setup variables and call the program loop
void	Rendering(Window &window) {
	Profiler	pr;
	Shader		shader(
		"shaders/vertex.glsl",
		"shaders/fragment.glsl",
		"shaders/geometry.glsl"
	);
	Shader		shader2(
		"shaders/vertex.glsl",
		"shaders/fragment.glsl"
	);
	shader.use();
	shader2.use();
	shader.use();
	shader.recompile();
	shader2.recompile();

	program_loop(window, pr);

	pr.logToFile("out");
}