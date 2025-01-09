# pragma once

/// Defines
# define COLOR_HEADER_CXX
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

	public:
		Window(int posX, int posY, int width, int height, const std::string &title);
		~Window();

		/// Getters

		GLFWwindow *getGLFWwindow() const;
		operator GLFWwindow *() const;
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
