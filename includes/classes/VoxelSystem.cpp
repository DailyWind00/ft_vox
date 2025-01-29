#include "VoxelSystem.hpp"

//// VoxelSystem class
/// Constructors & Destructors
VoxelSystem::VoxelSystem() : VoxelSystem(0) {}

VoxelSystem::VoxelSystem(const uint64_t &seed) {
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

	size_t maxVerticesPerChunk = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE; // Worst case
	VBOcapacity = MAX_CHUNKS * maxVerticesPerChunk * sizeof(DATA_TYPE);
	
	glBufferStorage(GL_ARRAY_BUFFER, VBOcapacity, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	VBOdata = glMapBufferRange(GL_ARRAY_BUFFER, 0, VBOcapacity, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	if (!VBOdata)
		throw std::runtime_error("VoxelSystem : Failed to map the VBO");

	glVertexAttribIPointer(0, 2, GL_UNSIGNED_INT, sizeof(DATA_TYPE), nullptr);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create the IB
	glGenBuffers(1, &IB);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IB);

	IBcapacity = MAX_CHUNKS * sizeof(DrawArraysIndirectCommand);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, IBcapacity, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

	// Create the SSBO
	glGenBuffers(1, &SSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);

	SSBOcapacity = MAX_CHUNKS * sizeof(SSBOData);
	glBufferData(GL_SHADER_STORAGE_BUFFER, SSBOcapacity, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// Create the chunks (to remove)
	createChunk({ 0,  0, 0});
	createChunk({ 0, -1, 0});
	createChunk({-1,  0, 0});
	// for (int i = 0; i < 100; i++)
	// 	createChunk({i, 0, i});
}

VoxelSystem::~VoxelSystem() {
	if (VBOdata)
		glUnmapBuffer(GL_ARRAY_BUFFER);

	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &IB);
	glDeleteBuffers(1, &SSBO);
	glDeleteVertexArrays(1, &VAO);

	for (chunkData &chunk : chunks)
		delete chunk.chunk;
}
/// ---



/// Private functions

// Check if the voxel at the given position is visible
// TODO: Culling techniques (take the camera position as parameter)
bool VoxelSystem::isVoxelVisible(const size_t &x, const size_t &y, const size_t &z, AChunk *data) {
	if (BLOCK_AT(data, x, y, z) == 0)
		return false;

	// Check if any neighboring voxel is empty / out of bounds
	if ((!x || !BLOCK_AT(data, x - 1, y, z) || x >= CHUNK_SIZE - 1 || !BLOCK_AT(data, x + 1, y, z))
		|| (!y || !BLOCK_AT(data, x, y - 1, z) || y >= CHUNK_SIZE - 1 || !BLOCK_AT(data, x, y + 1, z))
		|| (!z || !BLOCK_AT(data, x, y, z - 1) || z >= CHUNK_SIZE - 1 || !BLOCK_AT(data, x, y, z + 1)))
		return true;

	return false;
}

// Create/update the mesh of the given chunk and store it in the VBO
DrawArraysIndirectCommand	VoxelSystem::genMesh(AChunk *data) {
	std::vector<DATA_TYPE>	vertices;
	int count = 0;
	
	// Generate vertices for visible faces
	for (size_t x = 0; x < CHUNK_SIZE; ++x) {
		for (size_t y = 0; y < CHUNK_SIZE; ++y) {
			for (size_t z = 0; z < CHUNK_SIZE; ++z) {
				if (isVoxelVisible(x, y, z, data)) {
					DATA_TYPE data = 0;

					// Bitmask :
					// position = 15 bits (5 bits per axis)
					// face = 3 bits (1 bit per axis, use culling in GPU)
					// uv = 7 bits
					// length = 15 bits (5 bits per axis)

					// Encode position
					data |= (x & 0x1F);       // 5 bits for x
					data |= (y & 0x1F) << 5;  // 5 bits for y
					data |= (z & 0x1F) << 10; // 5 bits for z

					vertices.push_back(data);
					count++;
				}
			}
		}
	}
	if (vertices.empty())
		return {0, 0, 0, 0};

	if (VERBOSE)
		std::cout << "> Chunk created with " << count << " blocks" << std::endl;

	// Update the persistent mapped buffer
	size_t dataSize = vertices.size() * sizeof(DATA_TYPE);
	if (currentVertexOffset + dataSize > VBOcapacity)
		throw std::runtime_error("VBO overflow"); // Todo: reallocate the buffer
	
	std::memcpy(reinterpret_cast<DATA_TYPE *>(VBOdata) + currentVertexOffset, vertices.data(), dataSize);

	// Create the draw command
	DrawArraysIndirectCommand command = {
		(GLuint)vertices.size(),
		1,
		(GLuint)(currentVertexOffset),
		0
	};

	currentVertexOffset += vertices.size();

	std::cout << "Vertice count: " << command.verticeCount << std::endl;
	std::cout << "Offset: " << command.offset << std::endl;
	std::cout << "Current offset: " << currentVertexOffset << std::endl;

	return command;
}

// Create a chunk at the given world position
void	VoxelSystem::createChunk(const glm::ivec3 &worldPos) {
	AChunk *chunk = ChunkHandler::createChunk(worldPos);

	// Store the draw command for the chunk
	DrawArraysIndirectCommand command = genMesh(chunk);
	if (!command.verticeCount)
		return;
	commands.push_back(command);

	// Store the chunk data
	size_t offset = currentVertexOffset - command.verticeCount * 3 * sizeof(DATA_TYPE);
	size_t size = command.verticeCount * 3 * sizeof(DATA_TYPE) + offset;
	chunkData data = {chunk, worldPos, offset, size};
	chunks.push_back(data);

	// Update indirect buffer
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IB);
	if (commands.size() * sizeof(DrawArraysIndirectCommand) > IBcapacity) {
		IBcapacity *= 2;
		glBufferData(GL_DRAW_INDIRECT_BUFFER, IBcapacity, nullptr, GL_DYNAMIC_DRAW);
	}
	glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0, commands.size() * sizeof(DrawArraysIndirectCommand), commands.data());
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

	// Update SSBO
	SSBOData ssboData = {{worldPos.x, worldPos.y, worldPos.z, 0}};
	chunksInfos.push_back(ssboData);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
	if (chunksInfos.size() * sizeof(SSBOData) > SSBOcapacity) {
		SSBOcapacity *= 2;
		glBufferData(GL_SHADER_STORAGE_BUFFER, SSBOcapacity, nullptr, GL_DYNAMIC_DRAW);
	}
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, chunksInfos.size() * sizeof(SSBOData), chunksInfos.data());

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
/// ---



/// Public functions

// Draw all chunks using batched rendering
void	VoxelSystem::draw() const {
	// Todo: UpdateChunk here (may need to add parameters to the function)

	glBindVertexArray(VAO);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IB);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
	
	glMultiDrawArraysIndirect(GL_POINTS, nullptr, commands.size(), sizeof(DrawArraysIndirectCommand));
	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	glBindVertexArray(0);
}
/// ---
//// ----