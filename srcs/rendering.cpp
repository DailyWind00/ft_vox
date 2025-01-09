#include "config.hpp"

// Keep the window alive, exiting this function should mean closing the window
static void program_loop(Window &window, Profiler &pr) {
	(void)pr;
	while (!glfwWindowShouldClose(window)) {
		glfwSwapBuffers(window);

		Chunk	chunk = Chunk(1);

		handleEvents(window);
	}
}

// Setup variables and call the program loop
void	Rendering(Window &window) {
	Profiler	pr;

	std::cout << sizeof(ChunkNode) << std::endl;

	program_loop(window, pr);

	pr.logToFile("out");
}
