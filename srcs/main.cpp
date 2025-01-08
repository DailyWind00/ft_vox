#include "config.hpp"

// temporary (to quit for now)
# include <glfw/glfw3.h>
# include <unistd.h>
# include <stdio.h>

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

			std::string	str = "wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww";

			pr.evaluate("write", *write, 1, (const void *)str.c_str(), str.size());


			// Poll events
			glfwPollEvents();
		}
		pr.logToFile();
	}
	catch(const exception& e) {
		cerr << BRed <<  "Critical Error : " << e.what() << ResetColor <<'\n';
		exit(EXIT_FAILURE);
	}
}
