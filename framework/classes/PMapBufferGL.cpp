#include "PMapBufferGL.hpp"

/// Constructors & Destructors
PMapBufferGL::PMapBufferGL(GLenum type, size_t capacity, GLenum usage)
    : _type(type), _usage(usage), _capacity(capacity), _data(nullptr) {
	
	glGenBuffers(1, &_id);
    glBindBuffer(_type, _id);

    glBufferStorage(type, capacity, nullptr, PERSISTENT_BUFFER_USAGE);

    _data = glMapBufferRange(type, 0, capacity, _usage);

    if (!_data)
		throw std::runtime_error("PMapBufferGL: Failed to map the buffer");

	if (VERBOSE)
		std::cout << "Created PMapBufferGL with a capacity of " << capacity << " bytes" << std::endl;
}

PMapBufferGL::~PMapBufferGL() {
	if (_data)
		glUnmapBuffer(_type);

	if (VERBOSE)
		std::cout << "Destroyed PMapBufferGL\n";
}
/// ---



/// Public functions

// Bind the buffer
void PMapBufferGL::bind() {
	glBindBuffer(_type, _id);
}

// Unbind the buffer
void PMapBufferGL::unbind() {
	glBindBuffer(_type, 0);
}

// Flush the buffer (need GL_MAP_FLUSH_EXPLICIT_BIT flag)
// If length is 0, the whole buffer is flushed from the offset
void PMapBufferGL::flush(size_t offset, size_t length) const {
	if (!(_usage & GL_MAP_FLUSH_EXPLICIT_BIT))
		throw std::runtime_error("PMapBufferGL: Buffer not created with GL_MAP_FLUSH_EXPLICIT_BIT flag");

	if (offset >= _capacity)
		return;

	if (!length || offset + length > _capacity)
		length = _capacity - offset;

    glFlushMappedBufferRange(_type, offset, length);
}

// Reallocate the buffer with a new capacity (copy the data back if keepData is true)
// This function recreate a new buffer so don't forget to update your buffer attributes
size_t PMapBufferGL::resize(size_t newCapacity, bool keepData) {
	if (newCapacity < _capacity)
		return _capacity;

	// Copy current data
	void *copy = nullptr;
	if (keepData) {
		copy = new uint8_t[_capacity];
		std::memcpy(copy, _data, _capacity);
	}

	// Delete current buffer
	glUnmapBuffer(_type);
	glDeleteBuffers(1, &_id);

	// Create a new buffer
	glGenBuffers(1, &_id);
	glBindBuffer(_type, _id);
	glBufferStorage(_type, newCapacity, copy, PERSISTENT_BUFFER_USAGE);
    _data = glMapBufferRange(_type, 0, newCapacity, _usage);

	delete[] static_cast<uint8_t*>(copy);

    if (!_data)
        return _capacity = 0;

    return _capacity = newCapacity;
}

// Write data to the buffer at a given offset
// If src is nullptr, the data is filled with 0
bool PMapBufferGL::write(const void* src, size_t size, size_t offset) {
    if (offset + size > _capacity)
		return false;

	if (!src)
		std::memset(static_cast<uint8_t*>(_data) + offset, 0, size);
	else
		std::memcpy(static_cast<uint8_t*>(_data) + offset, src, size);

    return true;
}
/// ---



/// Getters

// Return the buffer data
const void * PMapBufferGL::getData() const {
    return _data;
}

// Return the buffer ID
const GLuint & PMapBufferGL::getID() const {
    return _id;
}

// Return the buffer type
const GLenum & PMapBufferGL::getType() const {
	return _type;
}

// Return the buffer capacity
const size_t & PMapBufferGL::getCapacity() const {
    return _capacity;
}
/// ---
