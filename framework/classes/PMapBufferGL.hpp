#pragma once

/// Defines
# define PERSISTENT_BUFFER_USAGE GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT

/// System includes
#include <iostream>
#include <cstring>

/// Dependencies
#include <glad/glad.h>

/// Global variables
extern bool VERBOSE;

// This class is a simple wrapper around OpenGL persistent mapped buffers
// Set GL_MAP_FLUSH_EXPLICIT_BIT if you want to flush the buffer manually
class PMapBufferGL {
	private:
		GLuint	_id;
		GLenum	_type;
		GLenum	_usage;
		size_t	_capacity;
		void *	_data;

	public:
		PMapBufferGL(GLenum type, size_t capacity, GLenum usage = 0);
		~PMapBufferGL();

		/// Public functions
		void    bind();
		void    unbind();
		size_t  reallocate(size_t newCapacity);
		bool    write(const void* src, size_t size, size_t offset = 0);
		void    flush(size_t offset = 0, size_t length = 0) const;
		void    sync(GLenum barrier) const;

		/// Getters
		const void *	getData() const;
		const GLuint &	getID() const;
		const size_t &	getCapacity() const;
};