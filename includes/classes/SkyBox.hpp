#pragma once

/// Defines
# define COLOR_HEADER_CXX

/// System includes
# include <iostream>

/// Dependencies
# include <glad/glad.h>
# include <glfw/glfw3.h>
# include "stb_image/stb_image.h"

/// Global variables
extern bool VERBOSE;

// Class for a simple skybox
class	SkyBox {
	private:
		GLuint	VAO;
		GLuint	VBO;
		GLuint	textureID;

		/// Private functions

		void	loadTexture(const std::string &path);

	public:
		SkyBox();
		~SkyBox();

		/// Public functions

		void	draw();
};
