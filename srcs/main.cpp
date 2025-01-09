#include "config.hpp"

// temporary (to quit for now)
# include <glfw/glfw3.h>
# include <unistd.h>

void	countTo(int num)
{
	int	i = 0;

	while (i < num)
		i++;
}

int main(int argc, char **argv) {
	(void)argc; (void)argv;

	try {
		// Create a window
		WindowsHandler windowsHandler;
		windowsHandler.createWindow(0, 0, 800, 600, "ft_vox");

		Profiler	pr;

		// Main loop
		while (!glfwWindowShouldClose(*windowsHandler.front())) {
			// // Rendering
			// Rendering(*windowsHandler[windowIndex]);

			// Swap buffers
			glfwSwapBuffers(*windowsHandler[0]);

			// temporary (to quit for now)
			VGLFWwindows::const_iterator	currWin = windowsHandler[0];
			if (glfwGetKey(*currWin, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
				windowsHandler.destroyWindow(0);
				break ;
			}

			pr.evaluateNoReturn("countTo100000000", &countTo, 100000000);
			pr.evaluateNoReturn("countTo10000000", &countTo, 10000000);
			pr.evaluateNoReturn("countTo1000000", &countTo, 1000000);
			pr.evaluateNoReturn("countTo100000", &countTo, 100000);
			pr.evaluateNoReturn("countTo10000", &countTo, 10000);
			pr.evaluateNoReturn("countTo1000", &countTo, 1000);


			// Poll events
			glfwPollEvents();
		}
		pr.logToFile("out.logs");
	}
	catch(const exception& e) {
		cerr << BRed <<  "Critical Error : " << e.what() << ResetColor <<'\n';
		exit(EXIT_FAILURE);
	}
}
