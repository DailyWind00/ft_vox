# pragma once

/// Defines
# define FAILURE -1

/// System includes
# include <string>
# include <vector>

/// Dependencies
# include <glad/glad.h>
# include <glfw/glfw3.h>

/// Global variables
extern bool VERBOSE;

class Window {
	friend class WindowsHandler;

	private:
		GLFWwindow *window;

		// Only used for the main loop
		size_t		fps = 0;
		double		frameTime = 0;

		/// Private functions

		void	updateFrameRate();

	public:
		Window(int posX, int posY, int width, int height, const std::string &title);
		~Window();

		/// Public functions

		// Main loop of the program
		// Handle the window events and call the given function
		template <typename ...T>
		void	mainLoop(void (*func)(T&...), T&... args) {
			if (VERBOSE)
				std::cout << "Window : Entering loop" << std::endl;

			while (!glfwWindowShouldClose(window)) {
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				this->updateFrameRate();

				func(args...);

				glfwPollEvents();
				glfwSwapBuffers(window);
			}

			if (VERBOSE)
				std::cout << "Window : Exiting loop" << std::endl;
		}

		/// Getters

		GLFWwindow *getGLFWwindow() const;
		operator	GLFWwindow *() const;

		size_t	getFPS();
		double	getFrameTime() const;

		/// Setters

		void	setTitle(const std::string &title);
};

typedef std::vector<Window *>   VGLFWwindows;

// Optionnal class for multiple windows handling
// Consider use multi-threading to handle multiple windows main loop
class WindowsHandler {
    private:
        VGLFWwindows    windows;

    public:
        WindowsHandler();
        ~WindowsHandler();

        /// Public functions

        int     createWindow(int posX, int posY, int width, int height, const std::string &title);
        void    useWindow(size_t index);
        void    destroyWindow(size_t index);
        void    destroyAllWindows();

        /// Getters

		int                             getIndexOf(Window *window) const;
        VGLFWwindows::const_iterator    operator[](const size_t &index) const;
        VGLFWwindows::const_iterator    begin() const;
        VGLFWwindows::const_iterator    front() const;
        VGLFWwindows::const_iterator    back() const;
        VGLFWwindows::const_iterator    end() const;
};
