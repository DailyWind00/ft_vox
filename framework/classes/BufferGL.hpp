#pragma once

/// Defines
# define COLOR_HEADER_CXX

/// System includes
# include <iostream>
# include <vector>

/// Dependencies
# include <glad/glad.h>
# include "color.h"

/// Global variables
extern bool VERBOSE;

// This class is a simple wrapper around OpenGL buffers
// It allows to create, bind, unbind, update and delete buffers
class BufferGL {
	private:
		GLuint	_id;
		GLenum	_type;
		GLenum	_usage;
		size_t	_capacity;

	public:
		BufferGL(GLenum type, GLenum usage, size_t size = 0, const void *data = nullptr);
		~BufferGL();

		/// Public functions

		void	bind();
		void	unbind();
		void	updateData(const void *data, size_t size, size_t offset);
		void	resize(size_t newSize, const void *data = nullptr);
		void	clear();

		/// Getters

		const GLuint &	getID() const;
		const GLenum &	getType() const;
		const size_t &	getCapacity() const;
		bool			isValid() const;
};