#include "config.hpp"

// Handle all keyboard & other events
void	handleEvents(Window &window) {
	glfwPollEvents();

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}