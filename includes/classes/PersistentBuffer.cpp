#include "PersistentBuffer.hpp"

/// Constructors & Destructors
PersistentBuffer::PersistentBuffer(const GLenum &bufferType, const size_t &capacity, const GLenum &usage) {
	this->bufferType = bufferType;
	this->capacity = capacity;
	this->usage = usage;

	glGenBuffers(1, &bufferID);
	glBindBuffer(bufferType, bufferID);

	glBufferStorage(bufferType, capacity, nullptr, usage);
	data = glMapBufferRange(bufferType, 0, capacity, usage);
	if (!data)
		throw std::runtime_error("PersistentBuffer : Failed to map the buffer");

	glBindBuffer(bufferType, 0);
}

PersistentBuffer::~PersistentBuffer() {
	if (data)
		glUnmapBuffer(bufferType);

	glDeleteBuffers(1, &bufferID);
}
/// ---



/// Public functions

// Reallocate the buffer with a new capacity
// Return the new capacity, or 0 if an error occured
const size_t	&PersistentBuffer::reallocate(const size_t &newCapacity) {
	void *copy = nullptr;

	if (newCapacity < capacity)
		return capacity;

	if (data) {
		copy = new uint8_t[capacity];
		std::memcpy(copy, data, capacity);
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

	// Reallocate the buffer
	if (newCapacity != capacity) {

		glDeleteBuffers(1, &bufferID);
		glGenBuffers(1, &bufferID);
		glBindBuffer(GL_ARRAY_BUFFER, bufferID);

		capacity = newCapacity;
		glBufferStorage(GL_ARRAY_BUFFER, capacity, nullptr, usage);
		data = glMapBufferRange(GL_ARRAY_BUFFER, 0, capacity, usage);
		
		if (!data) { // Failed to map the buffer
			if (copy)
				delete[] static_cast<uint8_t*>(copy);
			return 0;
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	// Copy the data back
	std::memcpy(data, copy, capacity);

	delete[] static_cast<uint8_t*>(copy);
}

// Bind the buffer
void	PersistentBuffer::bind() const {
	glBindBuffer(bufferType, bufferID);
}

// Unbind the buffer
void	PersistentBuffer::unbind() const {
	glBindBuffer(bufferType, 0);
}

// Flush the buffer from offset to offset + length
void	PersistentBuffer::flush(const size_t &offset, const size_t &length) const {
	glFlushMappedBufferRange(bufferType, offset, length);
}

// Sync the buffer using barrier
void	PersistentBuffer::sync(const GLenum &barrier) const {
	glMemoryBarrier(barrier);
}
/// ---


/// Getters

// Return a const pointer to the buffer data
const void	*PersistentBuffer::getData() const {
	return data;
}

// Return the buffer ID
const GLuint	&PersistentBuffer::getBufferID() const {
	return bufferID;
}

// Return the buffer capacity
const size_t	&PersistentBuffer::getCapacity() const {
	return capacity;
}
/// ---



/// Setters

// Write data to the buffer
// Return true if the write was successful, false otherwise
const bool	&PersistentBuffer::write(const void *src, const size_t &size, const size_t &offset) {
	if (offset + size > capacity)
		return false;
	
	this->bind();
	std::memcpy(static_cast<uint8_t*>(data) + offset, src, size);
	this->unbind();

	return true;
}
/// ---