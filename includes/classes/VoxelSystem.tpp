#include "VoxelSystem.hpp"

//// VoxelSystem class
/// Constructors & Destructors
template <typename dataType, size_t chunkSize>
VoxelSystem<dataType, chunkSize>::VoxelSystem() : VoxelSystem<dataType>(rand()) {}

template <typename dataType, size_t chunkSize>
VoxelSystem<dataType, chunkSize>::VoxelSystem(const uint64_t &seed) {
	Noise::setSeed(seed);
	createChunk({0, 0, 0});

	// Create the VAO
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Create and allocate the VBO with persistent mapping
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	size_t vboSize  = 1024 * 1024 * sizeof(dataType); // todo: calculate the size needed
	glBufferStorage(GL_ARRAY_BUFFER, vboSize, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	VBOdata = glMapBufferRange(GL_ARRAY_BUFFER, 0, vboSize, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

	// Create the IB
	glGenBuffers(1, &IB);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IB);

    size_t commandBufferSize = commands.size() * sizeof(DrawArraysIndirectCommand);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, commandBufferSize, nullptr, GL_DYNAMIC_DRAW);
}

template <typename dataType, size_t chunkSize>
VoxelSystem<dataType, chunkSize>::~VoxelSystem() {
	if (VBOdata)
		glUnmapBuffer(GL_ARRAY_BUFFER);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &IB);
	glDeleteVertexArrays(1, &VAO);

	for (chunkData &chunk : chunks)
		delete chunk.chunk;
}
/// ---



/// Private functions

// Check if the voxel at the given position is visible
template <typename dataType, size_t chunkSize>
bool VoxelSystem<dataType, chunkSize>::isVoxelVisible(const glm::ivec3 &pos, const chunkData &data) {
	(void)pos; (void)data;
	// Todo

	return false;
}

// Create/update the mesh of the given chunk and store it in the VBO
template <typename dataType, size_t chunkSize>
DrawArraysIndirectCommand	VoxelSystem<dataType, chunkSize>::genMesh(const chunkData &data) {
	std::vector<dataType>	vertices;

	// Generate vertices for visible faces
    for (size_t x = 0; x < chunkSize; ++x) {
        for (size_t y = 0; y < chunkSize; ++y) {
            for (size_t z = 0; z < chunkSize; ++z) {
				if (isVoxelVisible({x, y, z}, data))
					std::cout << "block at " << x << " " << y << " " << z << std::endl; // to remove
            }
        }
    }

	// Update the VBO
	size_t dataSize = vertices.size() * sizeof(dataType);
	std::memcpy(reinterpret_cast<uint8_t *>(VBOdata) + currentVertexOffset, vertices.data(), dataSize);

    // Create the draw command
	DrawArraysIndirectCommand command = {
		(GLuint)vertices.size(),
		1,
		(GLuint)(currentVertexOffset / sizeof(dataType)),
		0
	};
	currentVertexOffset += dataSize;
	return command;
}

// Create a chunk at the given world position
template <typename dataType, size_t chunkSize>
void	VoxelSystem<dataType, chunkSize>::createChunk(const glm::ivec3 &worldPos) {
	AChunk *chunk = new LayeredChunk(0);
	chunkData data = {chunk, worldPos};

	chunk->generate(worldPos);
	// chunk->print(); // to remove

	chunks.push_back(data);
	commands.push_back(this->genMesh(data)); // Generate the mesh and store the draw command

    // Update indirect buffer
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IB);
    glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0, commands.size() * sizeof(DrawArraysIndirectCommand), commands.data());
}
/// ---



/// Public functions

// Draw all chunks
template <typename dataType, size_t chunkSize>
void	VoxelSystem<dataType, chunkSize>::draw() const {
	// Todo: Update the meshes of the chunks (frustum culling, ...) (may need to add parameters to the function)

	glBindVertexArray(VAO);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IB);

    // Use glMultiDrawArraysIndirect for batched rendering
    glMultiDrawArraysIndirect(GL_TRIANGLES, commands.data(), commands.size(), sizeof(DrawArraysIndirectCommand));
}
/// ---
//// ----