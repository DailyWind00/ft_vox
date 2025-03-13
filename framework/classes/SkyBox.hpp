#pragma once

/// Defines
# define COLOR_HEADER_CXX

/// System includes
# include <iostream>
# include <vector>

/// Dependencies
# include <glad/glad.h>
# include <glfw/glfw3.h>
# include "stb_image/stb_image.h"
# include "color.h"

/// Global variables
extern bool VERBOSE;

// Class for a simple skybox
// Constructor takes a vector of 6 paths to the skybox textures
// The order should be:
// +X (right), -X (left), +Y (top), -Y (bottom), +Z (front), -Z (back)
class	SkyBox {
	private:
		GLuint	VAO;
		GLuint	VBO;
		GLuint	textureID = 0;

	public:
		SkyBox(const std::vector<std::string> &path = {});
		~SkyBox();

		/// Public functions

		void	draw();
};
