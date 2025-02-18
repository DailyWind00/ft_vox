# include <unistd.h>
# include <chrono>

# include "VoxelSystem.hpp"
# include "config.hpp"

//// VoxelSystem class
/// Constructors & Destructors
VoxelSystem::VoxelSystem() : VoxelSystem(0) {}

VoxelSystem::VoxelSystem(const uint64_t &seed) : updatingBuffers(false), quitting(false)
{
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

	// Default QuadVBO
	unsigned int	quadVBO = 0;
	float	quadVert[] = {
		0, 1, 0, 0, 0,
		0, 1, 1, 0, 1,
		1, 1, 0, 1, 0,
		1, 1, 1, 1, 1
	};

	glGenBuffers(1, &quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVert), quadVert, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Create and allocate the VBO with persistent mapping
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	size_t maxVerticesPerChunk = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE; // Impossible worst case (just to be sure)
	VBOcapacity = VERTICALE_RENDER_DISTANCE * HORIZONTALE_RENDER_DISTANCE * HORIZONTALE_RENDER_DISTANCE * maxVerticesPerChunk * sizeof(DATA_TYPE);
	
	if (VERBOSE)
		std::cout << "> Creating VBO with a capacity of " << VBOcapacity / sizeof(DATA_TYPE) << " blocks" << std::endl;

	glBufferStorage(GL_ARRAY_BUFFER, VBOcapacity, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	VBOdata = glMapBufferRange(GL_ARRAY_BUFFER, 0, VBOcapacity, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	if (!VBOdata)
		throw std::runtime_error("VoxelSystem : Failed to map the VBO");

	glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(DATA_TYPE), nullptr);
	glVertexAttribDivisor(1, 1);	

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create the IB
	glGenBuffers(1, &IB);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IB);

	IBcapacity = VERTICALE_RENDER_DISTANCE * HORIZONTALE_RENDER_DISTANCE * HORIZONTALE_RENDER_DISTANCE * 6 * sizeof(DrawCommand);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, IBcapacity, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

	// Create the SSBO
	glGenBuffers(1, &SSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);

	SSBOcapacity = VERTICALE_RENDER_DISTANCE * HORIZONTALE_RENDER_DISTANCE * HORIZONTALE_RENDER_DISTANCE * 6 * sizeof(SSBOData);
	glBufferData(GL_SHADER_STORAGE_BUFFER, SSBOcapacity, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glBindVertexArray(0);

	// Generating the GBuffer
	glGenFramebuffers(1, &this->gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, this->gBuffer);

	// Position color buffer
	glGenTextures(1, &this->gPosition);
	glBindTexture(GL_TEXTURE_2D, this->gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->gPosition, 0);

	// Normal color buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, this->gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, this->gNormal, 0);
  
	// Albedo color Buffer
	glGenTextures(1, &this->gColor);
	glBindTexture(GL_TEXTURE_2D, this->gColor);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, this->gColor, 0);

	// Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,  GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	unsigned int rboDepth;

	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WINDOW_WIDTH, WINDOW_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Load the texture Atlas
	int	w, h, nChannels;
	unsigned char	*atlasData = stbi_load("./assets/atlas.png", &w, &h, &nChannels, 0);

	// Generate the Texture Buffer Object
	glGenTextures(1, &this->textures);
	glBindTexture(GL_TEXTURE_2D, this->textures);
	
	// Sets up the texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, atlasData);

	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(atlasData);

	// VoxelSystem threads initialization
	this->meshGenThread = std::thread(&VoxelSystem::meshGenRoutine, this);
	this->chunkGenThread = std::thread(&VoxelSystem::chunkGenRoutine, this);

	if (VERBOSE)
		std::cout << "VoxelSystem initialized\n";
}

VoxelSystem::~VoxelSystem() {
	if (VBOdata)
		glUnmapBuffer(GL_ARRAY_BUFFER);

	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &IB);
	glDeleteBuffers(1, &SSBO);
	glDeleteVertexArrays(1, &VAO);

	this->quitting = true;

	// waiting for threads to finish
	this->meshGenThread.join();
	this->chunkGenThread.join();

	for (std::pair<glm::ivec3, AChunk *> chunk : chunks)
		delete chunk.second;
	chunks.clear();
	
	if (VERBOSE)
		std::cout << "VoxelSystem destroyed\n";
}
/// ---


/// Private functions

void	VoxelSystem::updateDrawCommands()
{
	this->VDrawCommandMutex.lock();
	
	for (DrawCommandData cmds : cmdData) {
		for (size_t i = 0; i < 6; i++) {
			// TODO: update already generated chunks meshes next to the new one
			size_t	dataSize = cmds.vertices[i].size() * sizeof(DATA_TYPE);	

			if (!cmds.vertices[i].size())
				continue ;
			std::memcpy(reinterpret_cast<DATA_TYPE *>(VBOdata) + cmds.cmd[i].baseInstance, cmds.vertices[i].data(), dataSize);

			this->commands.push_back(cmds.cmd[i]);
			chunksInfos.push_back({{cmds.wPos.x, cmds.wPos.y, cmds.wPos.z, i}});
		}
	}
	cmdData.clear();
	this->VDrawCommandMutex.unlock();
}

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
void	VoxelSystem::reallocateVBO(size_t newSize) 
{
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

		glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(DATA_TYPE), nullptr);
		glVertexAttribDivisor(1, 1);
		glEnableVertexAttribArray(1);
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
// Return a bitmask of the visible faces (6 bits : ZzYyXx)
uint8_t	VoxelSystem::isVoxelVisible(const size_t &x, const size_t &y, const size_t &z, const ChunkData &data, AChunk *neightboursChunks[6], const size_t &LOD)
{
	// check if the voxel is solid
	if (!BLOCK_AT(data.second, x, y, z))
		return (0);

	// define neightbouring blocks positions
	glm::ivec3	neightbours[6] = {
		{x - 1 * LOD, y, z},
		{x + 1 * LOD, y, z},
		{x, y - 1 * LOD, z},
		{x, y + 1 * LOD, z},
		{x, y, z - 1 * LOD},
		{x, y, z + 1 * LOD}
	};

	// Check if the face is visible
	// Also check if the neightbours are in the same chunk or in another one
	uint8_t	visibleFaces = 0;

	for (const glm::ivec3 &pos : neightbours) {
		/// X axis
		// Border
		if (pos.x < 0) {
			if (!neightboursChunks[4] || !BLOCK_AT(neightboursChunks[4], CHUNK_SIZE - LOD, pos.y, pos.z))
				visibleFaces |= (1 << 0);
		}
		else if (pos.x >= CHUNK_SIZE) {
			if (!neightboursChunks[5] || !BLOCK_AT(neightboursChunks[5], 0, pos.y, pos.z))
				visibleFaces |= (1 << 1);
		}
		// Inside
		else if (size_t(pos.x) < x && !BLOCK_AT(data.second, pos.x, pos.y, pos.z))
				visibleFaces |= (1 << 0);
		else if (size_t(pos.x) > x && !BLOCK_AT(data.second, pos.x, pos.y, pos.z))
				visibleFaces |= (1 << 1);


		/// y axis
		// Border
		if (pos.y < 0) {
			if (!neightboursChunks[2] || !BLOCK_AT(neightboursChunks[2], pos.x, CHUNK_SIZE - LOD, pos.z))
				visibleFaces |= (1 << 2);
		}
		else if (pos.y >= CHUNK_SIZE) {
			if (!neightboursChunks[3] || !BLOCK_AT(neightboursChunks[3], pos.x, 0, pos.z))
				visibleFaces |= (1 << 3);
		}
		// Inside
		else if (size_t(pos.y) < y && !BLOCK_AT(data.second, pos.x, pos.y, pos.z))
				visibleFaces |= (1 << 2);
		else if (size_t(pos.y) > y && !BLOCK_AT(data.second, pos.x, pos.y, pos.z))
				visibleFaces |= (1 << 3);


		/// z axis
		// Border
		if (pos.z < 0) {
			if (!neightboursChunks[0] || !BLOCK_AT(neightboursChunks[0], pos.x, pos.y, CHUNK_SIZE - LOD))
				visibleFaces |= (1 << 4);
		}
		else if (pos.z >= CHUNK_SIZE) {
			if (!neightboursChunks[1] || !BLOCK_AT(neightboursChunks[1], pos.x, pos.y, 0))
				visibleFaces |= (1 << 5);
		}
		// Inside
		else if (size_t(pos.z) < z && !BLOCK_AT(data.second, pos.x, pos.y, pos.z))
				visibleFaces |= (1 << 4);
		else if (size_t(pos.z) > z && !BLOCK_AT(data.second, pos.x, pos.y, pos.z))
				visibleFaces |= (1 << 5);
	}	

	return visibleFaces;
}

// Create/update the mesh of the given chunk and store it in the VBO
DrawCommandData	VoxelSystem::genMesh(const ChunkData &chunk, const size_t &LOD)
{
	// define neightbouring chunks positions
	glm::ivec3	neightbours[6] = { 
		{chunk.first.x - 1, chunk.first.y, chunk.first.z},
		{chunk.first.x + 1, chunk.first.y, chunk.first.z},
		{chunk.first.x, chunk.first.y - 1, chunk.first.z},
		{chunk.first.x, chunk.first.y + 1, chunk.first.z},
		{chunk.first.x, chunk.first.y, chunk.first.z - 1},
		{chunk.first.x, chunk.first.y, chunk.first.z + 1}
	};

	// search in the global chunk hash map for neightbouring chunks
	AChunk	*neightboursChunks[6] = {NULL};
	
	for (size_t i = 0; i < 6; i++) {
		if (chunks.find(neightbours[i]) != chunks.end())
			neightboursChunks[i] = chunks[neightbours[i]];
	}

	// Generate vertices for visible faces
	std::vector<DATA_TYPE>	vertices[6];

	for (size_t x = 0; x < CHUNK_SIZE; x += 1 * LOD) {
		// Check if there is any mesh needed in the chunk and skip if not
		if (IS_CHUNK_COMPRESSED(chunk.second) && !BLOCK_AT(chunk.second, 0, 0, 0))
			break ;

		for (size_t y = 0; y < CHUNK_SIZE; y += 1 * LOD) {
			// Check if there is any mesh needed in the layer and skip if not
			if (IS_LAYER_COMPRESSED(chunk.second, y) && !BLOCK_AT(chunk.second, 0, y, 0))
				continue ;

			// Creat a bit mask per face direction
			uint32_t	rowBitMasks[6] = {0};
			
			for (size_t z = 0; z < CHUNK_SIZE; z += 1 * LOD) {
				uint8_t visibleFaces = isVoxelVisible(x, y, z, chunk, neightboursChunks, LOD);

				for (int i = 0; i < 6; i++) {
					if (visibleFaces & (1 << i))
						rowBitMasks[i] |= (1 << z);
				}
			}

			// create the meshes from the bit masks
			for (int j = 0; j < 6; j++) {
				std::list<std::pair<uint8_t, uint8_t> >	lengths;
				std::pair<uint8_t, uint8_t>		currLen;
				
				// Create a list of the starts and ends of the faces in each rows
				for (int i = 0; i < 32; i++) {
					if (!i || !(rowBitMasks[j] & (1 << (i - 1))))
						currLen.first = i;

					if (!(rowBitMasks[j] & (1 << i)) && (rowBitMasks[j] & (1 << (i - 1)))) {
						lengths.push_back(currLen);
						currLen.second = 0;
					}
					
					if (rowBitMasks[j] & (1 << i))
						currLen.second++;
				}
				// clamp the data to avoid overflow
				if (currLen.second == 32)
					currLen.second = 31;

				// get the last value that has not been push in the list
				if (currLen.second)
					lengths.push_back(currLen);

				// fill the vertices buffers
				for (std::pair<uint8_t, uint8_t> l : lengths) {
					// Bitmask :
					// position = 15 bits (5 bits per 3D axis)
					// uv = 7 bits
					// length = 10 bits (5 bits per 2D axis) (greedy meshing)
					
					DATA_TYPE data = 0;
					
					// Encode position
					data |= (x & 0x1F);     	// 5 bits for x
					data |= (y & 0x1F) << 5;	// 5 bits for y
					data |= (l.first & 0x1F) << 10;	// 5 bits for z
					
					data |= (3 & 0x7F) << 15;	// 7 bits for block ID
			
					// Encode face length
					data |= (LOD * l.second & 0x1F) << 22; // 5 bits for length
					data |= (LOD * 1 & 0x1F) << 27; // 5 bits for length

					vertices[j].push_back(data);
				}
			}
		}
	}

	// Create a draw command for each face with data inside
	DrawCommandData	datas;
	
	for (int i = 0; i < 6; i++)
		datas.vertices[i] = vertices[i];
	for (int i = 0; i < 6; i++) {
		datas.cmd[i] = {4, (GLuint)vertices[i].size(), 0, (GLuint)currentVertexOffset};
		currentVertexOffset += vertices[i].size();
	}
	datas.wPos = chunk.first;
	return datas;
}

void	VoxelSystem::chunkGenRoutine()
{
	std::list<glm::ivec3>	localReqChunks;
	ChunkMap				localChunks;

	while (42) {
		if (this->quitting)
			break ;

		size_t	chunkBatchLimit = 0;

		// check for new chunks to be generated
		if (this->requestedChunks.size()) {
			this->requestedChunkMutex.lock();
			for (glm::ivec3 rc : this->requestedChunks) {
				if (chunkBatchLimit == 8192)
					break ;
				localReqChunks.push_back(rc);
				chunkBatchLimit++;
			}
			for (auto k = this->requestedChunks.begin(); chunkBatchLimit && this->requestedChunks.size(); chunkBatchLimit--) {
				this->requestedChunks.erase(k);
				k = this->requestedChunks.begin();
			}
			this->requestedChunkMutex.unlock();
		}

		size_t	count = 0;

		// generate requested chunks and temporary stores them
		for (glm::ivec3 chunkPos : localReqChunks) {
			AChunk	*chunk = ChunkHandler::createChunk(chunkPos);
			if (!chunk)
				continue ;

			localChunks[chunkPos] = chunk;
			count++;
		}
		localReqChunks.clear();

		if (!count) {
			usleep(1000 * 100);
			continue ;
		}

		// Stores new chunks in the VoxelSystem and adds them to the pendingChunks for their meshes to be generated
		this->pendingChunkMutex.lock();
		for (ChunkData chunk : localChunks) {
			this->chunks[chunk.first] = chunk.second;
			this->pendingChunks[chunk.first] = chunk.second;
		}
		this->pendingChunkMutex.unlock();
		localChunks.clear();
	}
}

void	VoxelSystem::meshGenRoutine()
{
	ChunkMap	localPendingChunks;
	
	while (42) {
		if (this->quitting)
			break ;

		// Check for chunks Mesh to be generated
		if (this->pendingChunks.size()) {
			this->pendingChunkMutex.lock();
			for (std::pair<glm::ivec3, AChunk *> chunk : this->pendingChunks)
				localPendingChunks[chunk.first] = chunk.second;

			this->pendingChunks.clear();
			this->pendingChunkMutex.unlock();
		}

		// Generate chunks meshes and creates their DrawCommands
		size_t	count = 0;

		if (this->VDrawCommandMutex.try_lock()) {
			for (std::pair<glm::ivec3, AChunk *> chunk : localPendingChunks) {
				// store draw commands and meshData
				DrawCommandData	datas;
				
				if (abs(chunk.first.x) * 0.1 >= 32 || abs(chunk.first.z) * 0.1 >= 32)
					datas = genMesh(chunk, 32);
				else if (abs(chunk.first.x) * 0.2 >= 16 || abs(chunk.first.z) * 0.2 >= 16)
					datas = genMesh(chunk, 16);
				else if (abs(chunk.first.x) * 0.3 >= 8 || abs(chunk.first.z) * 0.3 >= 8)
					datas = genMesh(chunk, 8);
				else if (abs(chunk.first.x) * 0.4 >= 4 || abs(chunk.first.z) * 0.4 >= 4)
					datas = genMesh(chunk, 4);
				else if (abs(chunk.first.x) * 0.5 >= 2 || abs(chunk.first.z) * 0.5 >= 2)
					datas = genMesh(chunk, 2);
				else
					datas = genMesh(chunk, 1);

				this->cmdData.push_back(datas);
				count++;
			}
			this->VDrawCommandMutex.unlock();
		}
		localPendingChunks.clear();

		// Check if any mesh has been created
		if (!count) {
			usleep(1000 * 100);
			continue ;
		}

		// Signal the main thread that it can update openGL's buffers
		this->updatingBuffers = true;
	}
}
/// ---


/// Public functions

void	VoxelSystem::requestChunk(const glm::ivec3 &start, const glm::ivec3 &end)
{
	if (!this->requestedChunkMutex.try_lock())
		return ;
	for (int i = start.x; i < end.x; i++)
		for (int j = start.y; j < end.y; j++)
			for (int k = start.z; k < end.z; k++)
				requestChunk({i, j, k}, true);
	this->requestedChunkMutex.unlock();
}

void	VoxelSystem::requestChunk(const glm::ivec3 &pos, const bool &batched)
{
	if (this->chunks[pos] || std::find(this->requestedChunks.begin(), this->requestedChunks.end(), pos) != this->requestedChunks.end())
		return ;

	if (!batched) {
		if (!this->requestedChunkMutex.try_lock())
			return ;
		this->requestedChunks.push_back(pos);
		this->requestedChunkMutex.unlock();
		return ;
	}
	std::cout << "requesting " << pos.x << " " <<  pos.y << " " << pos.z << std::endl;
	this->requestedChunks.push_back(pos);
}

// Draw all chunks using batched rendering
GeoFrameBuffers	VoxelSystem::draw() {
	if (this->updatingBuffers) {
		this->updateDrawCommands();
		this->updateIB();
		this->updateSSBO();
		this->updatingBuffers = false;
	}

	// Binding the gBuffer
	glBindFramebuffer(GL_FRAMEBUFFER, this->gBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// World DrawCall
	glBindVertexArray(VAO);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IB);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
        glBindTexture(GL_TEXTURE_2D, this->textures);

	glMultiDrawArraysIndirect(GL_TRIANGLE_STRIP, nullptr, commands.size(), sizeof(DrawCommand));

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	glBindVertexArray(0);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return (GeoFrameBuffers) {
		this->gBuffer,
		this->gPosition,
		this->gNormal,
		this->gColor
	};
}
/// ---
//// ----
