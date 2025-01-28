#include "VoxelSystem.hpp"

//// VoxelSystem class
/// Constructors & Destructors
template <typename dataType, size_t chunkSize>
VoxelSystem<dataType, chunkSize>::VoxelSystem() : VoxelSystem<dataType, chunkSize>(0) {}

template <typename dataType, size_t chunkSize>
VoxelSystem<dataType, chunkSize>::VoxelSystem(const uint64_t &seed) {
	if (!seed) {
		srand(time(nullptr));
		Noise::setSeed(rand());
	}
	else
		Noise::setSeed(seed);

	// Create the VAO
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Create and allocate the VBO with persistent mapping
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	size_t maxChunks = 256; // todo: dynamic based on render distance, FOV, ...
	size_t maxVerticesPerChunk = chunkSize * chunkSize * chunkSize; // Worst case
	VBOsize = maxChunks * maxVerticesPerChunk * sizeof(dataType);
	
	glBufferStorage(GL_ARRAY_BUFFER, VBOsize, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	VBOdata = glMapBufferRange(GL_ARRAY_BUFFER, 0, VBOsize, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	if (!VBOdata)
		throw std::runtime_error("VoxelSystem : Failed to map the VBO");

	// Set the vertex attributes (move to reallocateVBO)
	if (std::is_same<dataType, int>::value)
		glVertexAttribIPointer(0, 3, GL_INT, 3 * sizeof(dataType), nullptr);

	else if (std::is_same<dataType, unsigned int>::value)
		glVertexAttribIPointer(0, 3, GL_UNSIGNED_INT, 3 * sizeof(dataType), nullptr);

	else if (std::is_same<dataType, float>::value)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(dataType), nullptr);

	else if (std::is_same<dataType, double>::value)
		glVertexAttribLPointer(0, 3, GL_DOUBLE, 3 * sizeof(dataType), nullptr);

	else if (std::is_same<dataType, short>::value)
		glVertexAttribIPointer(0, 3, GL_SHORT, 3 * sizeof(dataType), nullptr);

	else if (std::is_same<dataType, unsigned short>::value)
		glVertexAttribIPointer(0, 3, GL_UNSIGNED_SHORT, 3 * sizeof(dataType), nullptr);

	else if (std::is_same<dataType, char>::value)
		glVertexAttribIPointer(0, 3, GL_BYTE, 3 * sizeof(dataType), nullptr);

	else if (std::is_same<dataType, unsigned char>::value)
		glVertexAttribIPointer(0, 3, GL_UNSIGNED_BYTE, 3 * sizeof(dataType), nullptr);

	else
		throw std::runtime_error("VoxelSystem : Unsupported data type");

	glEnableVertexAttribArray(0);

	// Create the IB
	glGenBuffers(1, &IB);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IB);

    IBsize = maxChunks * sizeof(DrawArraysIndirectCommand);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, IBsize, nullptr, GL_DYNAMIC_DRAW);

	// Create the chunks (to remove)
	createChunk({0, 0, 0});
	// for (int i = 0; i < 100; i++)
	// 	createChunk({i, 0, i});
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
// TODO: Culling techniques (take the camera position as parameter)
template <typename dataType, size_t chunkSize>
bool VoxelSystem<dataType, chunkSize>::isVoxelVisible(const size_t &x, const size_t &y, const size_t &z, AChunk *data) {
	if (BLOCK_AT(data, x, y, z) == 0)
		return false;

	// Check if any neighboring voxel is empty / out of bounds
	if ((!x || !BLOCK_AT(data, x - 1, y, z) || x >= chunkSize - 1 || !BLOCK_AT(data, x + 1, y, z))
		|| (!y || !BLOCK_AT(data, x, y - 1, z) || y >= chunkSize - 1 || !BLOCK_AT(data, x, y + 1, z))
		|| (!z || !BLOCK_AT(data, x, y, z - 1) || z >= chunkSize - 1 || !BLOCK_AT(data, x, y, z + 1)))
		return true;

	return false;
}

// Create/update the mesh of the given chunk and store it in the VBO
template <typename dataType, size_t chunkSize>
DrawArraysIndirectCommand	VoxelSystem<dataType, chunkSize>::genMesh(AChunk *data) {
	std::vector<dataType>	vertices;
	int count = 0;
	
	// Generate vertices for visible faces
    for (size_t x = 0; x < chunkSize; ++x) {
        for (size_t y = 0; y < chunkSize; ++y) {
            for (size_t z = 0; z < chunkSize; ++z) {
				if (isVoxelVisible(x, y, z, data)) {
					// TODO: use bitmasks
					vertices.push_back(x);
					vertices.push_back(y);
					vertices.push_back(z);
					count++;
				}
            }
        }
    }
	if (vertices.empty())
		return {0, 0, 0, 0};

	if (VERBOSE)
		std::cout << "> Chunk created with " << count << " vertices" << std::endl;

	// Update the persistent mapped buffer
	size_t dataSize = vertices.size() * sizeof(dataType);
	if (currentVertexOffset + dataSize > VBOsize)
		throw std::runtime_error("VBO overflow"); // Todo: reallocate the buffer
	
	std::memcpy(reinterpret_cast<uint8_t *>(VBOdata) + currentVertexOffset, vertices.data(), dataSize);

    // Create the draw command
	DrawArraysIndirectCommand command = {
		(GLuint)vertices.size() / 3,
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
	AChunk *chunk = ChunkHandler::createChunk(worldPos);

	// Store the draw command for the chunk
	DrawArraysIndirectCommand command = genMesh(chunk);
	if (!command.verticeCount)
		return;
	commands.push_back(command);

	// Store the chunk data
	size_t offset = currentVertexOffset - command.verticeCount * 3 * sizeof(dataType);
	size_t size = command.verticeCount * 3 * sizeof(dataType) + offset;
	chunkData data = {chunk, worldPos, offset, size};
	chunks.push_back(data);

    // Update indirect buffer
	if (commands.size() * sizeof(DrawArraysIndirectCommand) > IBsize) {
		IBsize *= 2;
		glBufferData(GL_DRAW_INDIRECT_BUFFER, IBsize, nullptr, GL_DYNAMIC_DRAW);
	}
	glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0, commands.size() * sizeof(DrawArraysIndirectCommand), commands.data());
}
/// ---



/// Public functions

// Draw all chunks
template <typename dataType, size_t chunkSize>
void	VoxelSystem<dataType, chunkSize>::draw() const {
	// Todo: UpdateChunk here (may need to add parameters to the function)

	glBindVertexArray(VAO);
	
	// Use glMultiDrawArraysIndirect for batched rendering
	glMultiDrawArraysIndirect(GL_POINTS, nullptr, commands.size(), sizeof(DrawArraysIndirectCommand));
}
/// ---
//// ----