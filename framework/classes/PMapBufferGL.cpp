#include "PMapBufferGL.hpp"

/// Constructors & Destructors
PMapBufferGL::PMapBufferGL(GLenum type, size_t capacity, GLenum usage)
    : _type(type), _usage(usage), _capacity(capacity), _data(nullptr) {
	
	glGenBuffers(1, &_id);
    glBindBuffer(_type, _id);

    glBufferStorage(type, capacity, nullptr, PERSISTENT_BUFFER_USAGE);

	_usage = usage | PERSISTENT_BUFFER_USAGE;
    _data = glMapBufferRange(type, 0, capacity, _usage);

    if (!_data)
        throw std::runtime_error("PMapBufferGL: Failed to map the buffer");
}

PMapBufferGL::~PMapBufferGL() {
    if (_data)
        glUnmapBuffer(_type);
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
	if (!length || offset + length > _capacity)
		length = _capacity - offset;

    glFlushMappedBufferRange(_type, offset, length);
}

// Synchronize the buffer with the GPU
void PMapBufferGL::sync(GLenum barrier) const {
    glMemoryBarrier(barrier);
}

// Reallocate the buffer with a new capacity (keeping the data)
size_t PMapBufferGL::reallocate(size_t newCapacity) {
	if (newCapacity < _capacity)
		return _capacity;

    void *copy = new uint8_t[_capacity];
	std::memcpy(copy, _data, _capacity);
	glUnmapBuffer(_type);

	_capacity = newCapacity;

	glDeleteBuffers(1, &_id);
	glGenBuffers(1, &_id);
	glBindBuffer(_type, _id);

	glBufferStorage(_type, newCapacity, copy, PERSISTENT_BUFFER_USAGE);
    _data = glMapBufferRange(_type, 0, newCapacity, PERSISTENT_BUFFER_USAGE | GL_MAP_FLUSH_EXPLICIT_BIT);

    if (!_data) {
        if (copy)
            delete[] static_cast<uint8_t*>(copy);

        return _capacity = 0;
    }
    
    std::memcpy(_data, copy, _capacity);
    delete[] static_cast<uint8_t*>(copy);

    return newCapacity;
}

// Write data to the buffer at a given offset
bool PMapBufferGL::write(const void* src, size_t size, size_t offset) {
    if (offset + size > _capacity)
        return false;

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
const GLuint & PMapBufferGL::getBufferID() const {
    return _id;
}

// Return the buffer capacity
const size_t & PMapBufferGL::getCapacity() const {
    return _capacity;
}
/// ---
