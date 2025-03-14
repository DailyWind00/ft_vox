/// Class independant system includes
# include <iostream>

#include "Window.hpp"

//// Window class
/// Constructors & Destructors
Window::Window(int posX, int posY, int width, int height, const std::string &title) {
	if (VERBOSE)
		std::cout << "Creating window" << std::endl;

	if (!glfwInit())
		throw std::runtime_error("Failed to initialize GLFW");
	if (VERBOSE)
		std::cout << "> GLFW initialized" << std::endl;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // OpenGL 4.6
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Mac-os compatibility
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	if (!window)
		throw std::runtime_error("Failed to create window");
	
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		glfwDestroyWindow(window);
		throw std::runtime_error("Failed to initialize GLAD");
	}
	if (VERBOSE)
		std::cout << "> GLAD initialized" << std::endl;

	glfwSetWindowPos(window, posX, posY);

	if (VERBOSE)
		std::cout << "Window created" << std::endl;
}

Window::~Window() {
	glfwDestroyWindow(window);
	glfwTerminate();

	if (VERBOSE)
		std::cout << "Window destroyed" << std::endl;
}
/// ---



/// Private functions

// Update the frame rate and the fps counters
void	Window::updateFrameRate() {
	static double	lastTime = glfwGetTime();
	static double	lastFrame = glfwGetTime();
	static size_t	frames = 0;

	frames++;

	// Update the frame time
	double currentTime = glfwGetTime();
	this->frameTime = (currentTime - lastFrame) * 1000.0;
	lastFrame = currentTime;

	// Update the fps counter every second
	if (currentTime - lastTime >= 1.0) {
		this->fps = frames;
		frames = 0;
		lastTime = currentTime;
	}
}
/// ---



/// Getters

// Return the GLFWwindow pointer
GLFWwindow	*Window::getGLFWwindow() const {
	return window;
}

// Return the GLFWwindow pointer
Window::operator GLFWwindow *() const {
	return window;
}

// Return the current FPS of the window
size_t	Window::getFPS() {
	return fps;
}

// Return the frameTime of the window
double	Window::getFrameTime() const {
	return frameTime;
}
/// ---



/// Setters

// Set the title of the window
void	Window::setTitle(const std::string &title) {
	glfwSetWindowTitle(window, title.c_str());
}
/// ---
//// ----





//// WindowHandler class
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
int	WindowsHandler::createWindow(int posX, int posY, int width, int height, const std::string &title) {
	windows.push_back( new Window(posX, posY, width, height, title) );

	return windows.size() - 1;
}

// Set the window at the given index as the current context
void	WindowsHandler::useWindow(size_t index) {
	if (index >= windows.size())
		return;
	glfwMakeContextCurrent(windows[index]->window);
}

// Destroy the window at the given index
// /!\ This function call std::vector::erase, which can invalidate previously saved iterators/indexes past the erased element
void	WindowsHandler::destroyWindow(size_t index) {
	if (index >= windows.size())
		return;
	delete windows[index];
	windows.erase(windows.begin() + index);
}

// Destroy all windows
void	WindowsHandler::destroyAllWindows() {
	for (Window *window : windows)
		delete window;
	windows.clear();
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

// Return the index of the given window, or FAILURE (-1) if the window is not found
int	WindowsHandler::getIndexOf(Window *window) const {
	for (size_t i = 0; i < windows.size(); i++) {
		if (windows[i] == window)
			return i;
	}
	return FAILURE;
}
/// ---
//// ----
