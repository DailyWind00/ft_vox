#include "config.hpp"

// Keep the window alive, exiting this function should mean closing the window
static void program_loop(Window &window, Profiler &pr) {
	(void)pr;
	while (!glfwWindowShouldClose(window)) {
		glfwSwapBuffers(window);

		handleEvents(window);
	}
}

static void	test()
{
	AChunk	*chunk = ChunkHandler::createChunk(glm::ivec3{0, -2, 0});
	chunk->print();
	delete chunk;
}

// Setup variables and call the program loop
void	Rendering(Window &window) {
	Profiler	pr;

	for (int i = 0; i < 1; i++)
		pr.evaluateNoReturn("LayeredChunk", &test);

	program_loop(window, pr);

	pr.logToFile("out");
}
