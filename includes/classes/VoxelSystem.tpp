#include "VoxelSystem.hpp"

//// VoxelSystem class
/// Constructors & Destructors
template <typename dataType>
VoxelSystem<dataType>::VoxelSystem() : VoxelSystem<dataType>(rand()) {}

template <typename dataType>
VoxelSystem<dataType>::VoxelSystem(const uint64_t &seed) {
	Noise::setSeed(seed);
	// createChunk({0, 0, 0});

	// Create the VAO
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Create and allocate the VBO with persistent mapping
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// size_t vboSize  = 1024 * 1024 * sizeof(dataType); // todo: calculate the size needed
	// glBufferStorage(GL_ARRAY_BUFFER, vboSize, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT); // segfault
	// VBOdata = glMapBufferRange(GL_ARRAY_BUFFER, 0, vboSize, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

	// Create the IB
	glGenBuffers(1, &IB);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IB);

    size_t commandBufferSize = commands.size() * sizeof(DrawArraysIndirectCommand);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, commandBufferSize, nullptr, GL_DYNAMIC_DRAW);
}

template <typename dataType>
VoxelSystem<dataType>::~VoxelSystem() {
	if (VBOdata)
		glUnmapBuffer(GL_ARRAY_BUFFER);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &IB);
	glDeleteVertexArrays(1, &VAO);
}
/// ---

/// Public functions

// Create a chunk at the given world position
template <typename dataType>
void	VoxelSystem<dataType>::createChunk(const glm::ivec3 &worldPos) {
	static size_t i = 0; // Not sure it's useful

	AChunk *chunk = new LayeredChunk(i++);
	chunk->generate(worldPos);
	chunk->print();

	chunks.push_back((chunkData){chunk, worldPos});
	// commands.push_back((DrawArraysIndirectCommand){ // need chunk->getVerticeCount() method
	// 	chunk->getVerticeCount(), 
	// 	1,
	// 	currentVertexOffset / (sizeof(dataType) * 3), // Offset in vertices, not bytes
	// 	commands.size()
	// });
    // currentVertexOffset += chunk->getVerticeCount() * sizeof(dataType) * 3;

    // Update indirect buffer
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IB);
    glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0, commands.size() * sizeof(DrawArraysIndirectCommand), commands.data());
}

// Draw all chunks
template <typename dataType>
void	VoxelSystem<dataType>::draw() const {
	glBindVertexArray(VAO);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IB);

    // Use glMultiDrawArraysIndirect for batched rendering
    glMultiDrawArraysIndirect(GL_TRIANGLES, nullptr, commands.size(), sizeof(DrawArraysIndirectCommand));
}
/// ---
//// ----