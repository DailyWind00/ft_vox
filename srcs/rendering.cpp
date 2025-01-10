#include "config.hpp"

// Keep the window alive, exiting this function should mean closing the window
static void program_loop(Window &window, Profiler &pr) {
	(void)pr;
	while (!glfwWindowShouldClose(window)) {
		glfwSwapBuffers(window);

		handleEvents(window);
	}
}

// Setup variables and call the program loop
void	Rendering(Window &window) {
	Profiler	pr;
	AChunk	* chunk = new LayeredChunk();
	(void)chunk;

	program_loop(window, pr);

	pr.logToFile("out");
}
