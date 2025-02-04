#pragma once

/// Defines
# define COLOR_HEADER_CXX

/// System includes
# include <iostream>
# include <cstring>

/// Dependencies
# include <glad/glad.h>

/// Global variables
extern bool VERBOSE;

// Data structure for a OpenGL persistent mapped buffer
// You need a pointer to this class to use it
// You must keep track of the data passed in from outside the class
class PMapBuffer {
	private:
		GLenum	bufferType;
		GLenum	usage;

		// Persistent buffer data
		GLuint		bufferID;
		void		*data;
		size_t		capacity;

	public:
		PMapBuffer(
			const GLenum &bufferType,
			const size_t &capacity,
			const GLenum &usage = GL_DYNAMIC_DRAW
		);
		~PMapBuffer();

		/// Public functions

		void			bind() const;
		void			unbind() const;
		size_t			reallocate(const size_t &newCapacity);
		void			flush(const size_t &offset = 0, const size_t &length = 0) const;
		void			sync(const GLenum &barrier = GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT) const;

		/// Getters

		const GLuint   &getBufferID() const;
		const void	   *getData() const;
		const size_t   &getCapacity() const;

		/// Setters

		bool		write(const void *src, const size_t &size, const size_t &offset = 0);
};