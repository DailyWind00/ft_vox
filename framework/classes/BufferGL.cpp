# include "BufferGL.hpp"

/// Constructors & Destructors
BufferGL::BufferGL()
	: _id(0), _type(0), _usage(0), _capacity(0) {
}


BufferGL::BufferGL(GLenum type, GLenum usage, size_t size, const void *data)
    : _type(type), _usage(usage), _capacity(size) {
    glGenBuffers(1, &_id);
    glBindBuffer(_type, _id);
    glBufferData(_type, size, data, _usage);
}


BufferGL::~BufferGL() {
	if (_id)
		glDeleteBuffers(1, &_id);
}
/// ---



/// Public functions

// Bind the buffer
void	BufferGL::bind() {
    if (!isValid()) {
		std::cerr << BOrange << "[BufferGL] Warning: Attempting to bind an uninitialized buffer (ID = 0, Type: " 
			<< _type << ")\n" << ResetColor;
		return;
	}
	
	glBindBuffer(_type, _id);
}

// Unbind the buffer
void	BufferGL::unbind() {
	glBindBuffer(_type, 0);
}

// Update the buffer with the given data at a given offset (in bytes)
void	BufferGL::updateData(const void *data, size_t size, size_t offset) {
    if (!isValid())
        throw std::runtime_error("BufferGL: Cannot update data on an uninitialized buffer.");
		
	if (size + offset > _capacity)
		throw std::out_of_range("BufferGL: Data overflow - Offset: " + std::to_string(offset) + 
								", Size: " + std::to_string(size) + 
								", Capacity: " + std::to_string(_capacity));
		
	bind();
	glBufferSubData(_type, offset, size, data);
}

// Resize the buffer with the given size and update it with the given data
// This function is heavier than updateData, prefer using updateData if you can
void BufferGL::resize(size_t newSize, const void *data) {
    if (!isValid()) {
        glGenBuffers(1, &_id);
        if (_type == 0 || _usage == 0)
            throw std::runtime_error("BufferGL: Cannot resize an uninitialized buffer without a type and usage.");
    }

    _capacity = newSize;

    bind();
    glBufferData(_type, newSize, data, _usage);
}


// Clear the buffer
void	BufferGL::clear() {
	if (!isValid() || _capacity == 0)
		return;

	bind();
	glBufferData(_type, _capacity, nullptr, _usage);
}
/// ---



/// Getters

// Return the buffer ID
const GLuint	&BufferGL::getID() const {
	return _id;
}

// Return the buffer type
const GLenum	&BufferGL::getType() const {
	return _type;
}

// Return the buffer capacity
const size_t	&BufferGL::getCapacity() const {
	return _capacity;
}

// Check if the buffer is valid
bool BufferGL::isValid() const {
    return _id != 0;
}
/// ---