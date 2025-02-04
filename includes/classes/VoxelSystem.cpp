#include "VoxelSystem.hpp"

//// VoxelSystem class
/// Constructors & Destructors
VoxelSystem::VoxelSystem() : VoxelSystem(0) {}

VoxelSystem::VoxelSystem(const uint64_t &seed) {
	if (VERBOSE)
		std::cout << "Creating VoxelSystem\n";

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

	size_t maxVerticesPerChunk = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE; // Impossible worst case (just to be sure)
	VBOcapacity = BASE_MAX_CHUNKS * maxVerticesPerChunk * sizeof(DATA_TYPE);
	
	if (VERBOSE)
		std::cout << "> Creating VBO with a capacity of " << VBOcapacity / sizeof(DATA_TYPE) << " blocks" << std::endl;

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

	IBcapacity = BASE_MAX_CHUNKS * sizeof(DrawCommand);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, IBcapacity, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

	// Create the SSBO
	glGenBuffers(1, &SSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);

	SSBOcapacity = BASE_MAX_CHUNKS * sizeof(SSBOData);
	glBufferData(GL_SHADER_STORAGE_BUFFER, SSBOcapacity, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// Create the chunks (to remove)
	
	for (int i = -10; i < 10; i++) {
		for (int j = -10; j < 10; j++) {
			for (int k = -1; k < 1; k++) {
				createChunk({i, k, j});
			}
		}
	}

	glBindVertexArray(0);

	if (VERBOSE)
		std::cout << "VoxelSystem created\n";
}

VoxelSystem::~VoxelSystem() {
	if (VBOdata)
		glUnmapBuffer(GL_ARRAY_BUFFER);

	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &IB);
	glDeleteBuffers(1, &SSBO);
	glDeleteVertexArrays(1, &VAO);

	for (ChunkData &chunk : chunks)
		delete chunk.chunk;
}
/// ---



/// Private functions

// Update the indirect buffer
void	VoxelSystem::updateIB() {
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IB);
	if (commands.size() * sizeof(DrawCommand) > IBcapacity) {
		IBcapacity *= BUFFER_GROWTH_FACTOR;
		glBufferData(GL_DRAW_INDIRECT_BUFFER, IBcapacity, nullptr, GL_DYNAMIC_DRAW);
	}
	glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0, commands.size() * sizeof(DrawCommand), commands.data());
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
}

// Update the SSBO
void	VoxelSystem::updateSSBO() {
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
	if (chunksInfos.size() * sizeof(SSBOData) > SSBOcapacity) {
		SSBOcapacity *= BUFFER_GROWTH_FACTOR;
		glBufferData(GL_SHADER_STORAGE_BUFFER, SSBOcapacity, nullptr, GL_DYNAMIC_DRAW);
	}
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, chunksInfos.size() * sizeof(SSBOData), chunksInfos.data());
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

// Reallocate the VBO with a new capacity, also recompact the data
void	VoxelSystem::reallocateVBO(size_t newSize) {
	void *copy = nullptr;

	size_t VBOstorage = 0;
	for (DrawCommand command : commands)
		VBOstorage += command.verticeCount * sizeof(DATA_TYPE);

	if (newSize < VBOstorage)
		throw std::runtime_error("VoxelSystem : New size is too small");

	if (VBOdata) {
		copy = new DATA_TYPE[VBOstorage / sizeof(DATA_TYPE)];
		std::memcpy(copy, VBOdata, VBOstorage);
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

	// Reallocate the buffer
	if (newSize != VBOcapacity) {
		if (VERBOSE)
			std::cout << "VoxelSystem : Reallocating VBO from " << VBOcapacity / sizeof(DATA_TYPE) << " to " << newSize / sizeof(DATA_TYPE) << " blocks\n";
		
		glDeleteBuffers(1, &VBO);
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		VBOcapacity = newSize;
		glBufferStorage(GL_ARRAY_BUFFER, VBOcapacity, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
		VBOdata = glMapBufferRange(GL_ARRAY_BUFFER, 0, VBOcapacity, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
		if (!VBOdata)
			throw std::runtime_error("VoxelSystem : Failed to map the VBO");

		glVertexAttribIPointer(0, 2, GL_UNSIGNED_INT, sizeof(DATA_TYPE), nullptr);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	// Update the draw commands and repopulate the buffer
	size_t newVertexOffset = 0;
	for (DrawCommand command : commands) {
		size_t dataSize = command.verticeCount * sizeof(DATA_TYPE);

		std::memcpy(
			reinterpret_cast<DATA_TYPE *>(VBOdata) + newVertexOffset,
			reinterpret_cast<DATA_TYPE *>(copy) + command.offset,
			dataSize
		);

		command.offset = newVertexOffset;
		newVertexOffset += command.verticeCount;
	}
	currentVertexOffset = newVertexOffset;

	delete[] static_cast<uint8_t*>(copy);
}

// Check if the voxel at the given position is visible
// TODO: Culling techniques (take the camera position as parameter)
bool VoxelSystem::isVoxelVisible(const size_t &x, const size_t &y, const size_t &z, AChunk *data) {
	if (BLOCK_AT(data, x, y, z) == 0)
		return false;

	// Check if any neighboring voxel is empty / out of bounds
	if (!x || !BLOCK_AT(data, x - 1, y, z) || x >= CHUNK_SIZE - 1 || !BLOCK_AT(data, x + 1, y, z))
		return true;
	if (!y || !BLOCK_AT(data, x, y - 1, z) || y >= CHUNK_SIZE - 1 || !BLOCK_AT(data, x, y + 1, z))
		return true;
	if (!z || !BLOCK_AT(data, x, y, z - 1) || z >= CHUNK_SIZE - 1 || !BLOCK_AT(data, x, y, z + 1))
		return true;

	return false;
}

// Create/update the mesh of the given chunk and store it in the VBO
DrawCommand	VoxelSystem::genMesh(AChunk *data) {
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

	// Update the persistent mapped buffer
	size_t dataSize = vertices.size() * sizeof(DATA_TYPE);
	while (currentVertexOffset + dataSize > VBOcapacity)
		reallocateVBO(VBOcapacity * 2);
	
	std::memcpy(reinterpret_cast<DATA_TYPE *>(VBOdata) + currentVertexOffset, vertices.data(), dataSize);

	// Create the draw command
	DrawCommand command = {
		(GLuint)vertices.size(),
		1,
		(GLuint)(currentVertexOffset),
		0
	};

	currentVertexOffset += vertices.size();

	return command;
}

// Create a chunk at the given world position
void	VoxelSystem::createChunk(const glm::ivec3 &worldPos) {
	AChunk *chunk = ChunkHandler::createChunk(worldPos);

	// Store the draw command for the chunk
	DrawCommand command = genMesh(chunk);
	if (!command.verticeCount)
		return;

	commands.push_back(command);
	chunks.push_back({chunk, worldPos});
	chunksInfos.push_back({{worldPos.x, worldPos.y, worldPos.z, 0}});

	this->updateIB();
	this->updateSSBO();
}

// Update the chunk at the given world position
void	VoxelSystem::updateChunk(const glm::ivec3 &worldPos) {
	size_t index = 0;
	bool found = false;
	for (ChunkData &chunk : chunks) {
		if (chunk.worldPos == worldPos) {
			deleteChunk(worldPos);
			createChunk(worldPos);

			found = true;
			break;
		}
		index++;
	}
	if (!found) return;

	this->updateIB();
	this->updateSSBO();
}

// Delete the chunk at the given world position
void	VoxelSystem::deleteChunk(const glm::ivec3 &worldPos) {
	size_t index = 0;
	bool found = false;
	for (ChunkData &chunk : chunks) {
		if (chunk.worldPos == worldPos) {
			delete chunk.chunk;

			chunks.erase(chunks.begin() + index);
			commands.erase(commands.begin() + index);
			chunksInfos.erase(chunksInfos.begin() + index);

			found = true;
			break;
		}
		index++;
	}
	if (!found) return;

	// Recompact the VBO
	size_t newVertexOffset = 0;
	for (DrawCommand &command : commands) {
		size_t dataSize = command.verticeCount * sizeof(DATA_TYPE);

		// Only copy if there's data to shift
		if (newVertexOffset != command.offset) {
			std::memmove(
				reinterpret_cast<DATA_TYPE *>(VBOdata) + newVertexOffset,
				reinterpret_cast<DATA_TYPE *>(VBOdata) + command.offset,
				dataSize
			);
		}

		command.offset = newVertexOffset;
		newVertexOffset += command.verticeCount;
	}
	
	this->updateIB();
	this->updateSSBO();
}
/// ---



/// Public functions

// Draw all chunks using batched rendering
void	VoxelSystem::draw() {
	// Todo: UpdateChunk here (may need to add parameters to the function)

	glBindVertexArray(VAO);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IB);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
	
	glMultiDrawArraysIndirect(GL_POINTS, nullptr, commands.size(), sizeof(DrawCommand));
	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	glBindVertexArray(0);
}
/// ---
//// ----
