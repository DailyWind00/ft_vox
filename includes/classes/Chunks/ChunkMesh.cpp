# include <ChunkMesh.hpp>

ChunkMesh::ChunkMesh(const std::vector<DATA_TYPE> &data) : _data(data), _dataSize(data.size() * sizeof(DATA_TYPE)) {
}

ChunkMesh::~ChunkMesh() {
	glDeleteVertexArrays(1, &_VAO);
	delete _VBO;
}

const GLuint &	ChunkMesh::getVAO() {
	return _VAO;
}

void	ChunkMesh::markToDelete() {
	_toDelete = true;
}

const bool &	ChunkMesh::checkToDelete() {
	return _toDelete;
}

void	ChunkMesh::updateMesh() {
	// Generate the chunk's vertex array
	glGenVertexArrays(1, &_VAO);
	glBindVertexArray(_VAO);

	// Generate the chunk's vertex buffer
	_VBO = new BufferGL(GL_ARRAY_BUFFER, GL_STATIC_DRAW, _dataSize, _data.data());

	glVertexAttribIPointer(0, 2, GL_UNSIGNED_INT, sizeof(uint32_t) * 2, nullptr);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}

void	ChunkMesh::draw() {
	glBindVertexArray(_VAO);
	glDrawArrays(GL_TRIANGLES, 0, _dataSize / sizeof(unsigned int));
	glBindVertexArray(0);
}
