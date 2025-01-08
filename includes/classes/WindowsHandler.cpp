/// Class independant system includes
# include <iostream>

#include "WindowsHandler.hpp"

/// Constructors & Destructors
WindowsHandler::WindowsHandler() {
	if (!glfwInit())
		throw std::runtime_error("Failed to initialize GLFW");
}

WindowsHandler::~WindowsHandler() {
	destroyAllWindows();
	glfwTerminate();
}
/// ---



/// Public functions

// Create a window, set it as the current context and add it to the windows vector
// Consider use multi-threading to handle multiple windows main loop
// Return the index of the new window, or FAILURE if the window creation failed
int	WindowsHandler::createWindow(int posX, int posY, int width, int height, const std::string &title) {
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // OpenGL 4.2
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Mac-os compatibility
	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow *window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	if (!window)
		return FAILURE;

	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		glfwDestroyWindow(window);
		return FAILURE;
	}

	glfwSetWindowPos(window, posX, posY);
	glViewport(0, 0, width, height);

	windows.push_back(window);

	if (VERBOSE)
		std::cout << "Window " << windows.size() - 1 << " created" << std::endl;

	return windows.size() - 1;
}

// Set the window at the given index as the current context
void	WindowsHandler::useWindow(size_t index) {
	if (index >= windows.size())
		return;
	glfwMakeContextCurrent(windows[index]);

	if (VERBOSE)
		std::cout << "Now using window " << index << std::endl;
}

// Destroy the window at the given index
// /!\ This function call std::vector::erase, which can invalidate previously saved iterators/indexes past the erased element
void	WindowsHandler::destroyWindow(size_t index) {
	if (index >= windows.size())
		return;
	glfwDestroyWindow(windows[index]);
	windows.erase(windows.begin() + index);

	if (VERBOSE)
		std::cout << "Window " << index << " destroyed" << std::endl;
}

// Destroy all windows
void	WindowsHandler::destroyAllWindows() {
	for (GLFWwindow *window : windows)
		glfwDestroyWindow(window);
	windows.clear();

	if (VERBOSE)
		std::cout << "All windows destroyed" << std::endl;
}
/// ---



/// Getters

// Return the window at the given index
VGLFWwindows::const_iterator	WindowsHandler::operator[](const size_t &index) const {
	if (index >= windows.size())
		return windows.cend();
	return windows.cbegin() + index;
}

// Return the begin of the windows vector
VGLFWwindows::const_iterator	WindowsHandler::begin() const {
	return windows.cbegin();
}

// Return the first window
VGLFWwindows::const_iterator	WindowsHandler::front() const {
	return windows.cbegin();
}

// Return the last window
VGLFWwindows::const_iterator	WindowsHandler::back() const {
	return windows.cend() - 1;
}

// Return the end of the windows vector
VGLFWwindows::const_iterator	WindowsHandler::end() const {
	return windows.cend();
}

// Return the index of the given window, or FAILURE if the window is not found
int	WindowsHandler::getIndexOf(GLFWwindow *window) const {
	for (size_t i = 0; i < windows.size(); i++) {
		if (windows[i] == window)
			return i;
	}
	return FAILURE;
}
/// ---
