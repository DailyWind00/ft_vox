# include <unistd.h>
# include <chrono>

# include "VoxelSystem.hpp"
# include "config.hpp"

//// VoxelSystem class
/// Constructors & Destructors
VoxelSystem::VoxelSystem() : VoxelSystem(0) {}

VoxelSystem::VoxelSystem(const uint64_t &seed) : camera(nullptr), updatingBuffers(false), quitting(false)
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
	size_t maxVerticesPerChunk = (glm::pow(CHUNK_SIZE, 3) / 2 + (CHUNK_SIZE % 2)) * 6; // Worst case scenario
	size_t VBOcapacity = VERTICALE_RENDER_DISTANCE * glm::pow(HORIZONTALE_RENDER_DISTANCE, 2) * maxVerticesPerChunk * sizeof(DATA_TYPE);

	if (VERBOSE)
		std::cout << "> VBO  : ";
	VBO = new PMapBufferGL(GL_ARRAY_BUFFER, VBOcapacity);

	glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(DATA_TYPE), nullptr);
	glVertexAttribDivisor(1, 1);	
	glEnableVertexAttribArray(1);

	// Create IB
	size_t IBcapacity = VERTICALE_RENDER_DISTANCE * glm::pow(HORIZONTALE_RENDER_DISTANCE, 2) * 6 * sizeof(DrawCommand);

	if (VERBOSE)
		std::cout << "> IB   : ";
	IB = new BufferGL(GL_DRAW_INDIRECT_BUFFER, GL_DYNAMIC_DRAW, IBcapacity);

	// Create SSBO
	size_t SSBOcapacity = VERTICALE_RENDER_DISTANCE * glm::pow(HORIZONTALE_RENDER_DISTANCE, 2) * 6 * sizeof(SSBOData);

	if (VERBOSE)
		std::cout << "> SSBO : ";
	SSBO = new BufferGL(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, SSBOcapacity);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO->getID());

	glBindVertexArray(0);

	// Generating the GBuffer
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	// Position color buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	// Normal color buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
  
	// Albedo color Buffer
	glGenTextures(1, &gColor);
	glBindTexture(GL_TEXTURE_2D, gColor);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gColor, 0);

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
	glGenTextures(1, &textures);
	glBindTexture(GL_TEXTURE_2D, textures);
	
	// Sets up the texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, atlasData);

	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(atlasData);

	// VoxelSystem threads initialization
	meshGenThread = std::thread(&VoxelSystem::meshGenRoutine, this);
	chunkGenThread = std::thread(&VoxelSystem::chunkGenRoutine, this);
	
	//-requestChunk({-10, -5, -10}, {10, 5, 10});

	if (VERBOSE)
		std::cout << "VoxelSystem initialized\n";
}

VoxelSystem::~VoxelSystem() {
	delete VBO;
	delete IB;
	delete SSBO;
	glDeleteVertexArrays(1, &VAO);

	quitting = true;

	// waiting for threads to finish
	meshGenThread.join();
	chunkGenThread.join();

	for (std::pair<glm::ivec3, AChunk *> chunk : chunks)
		delete chunk.second;
	chunks.clear();
	
	if (VERBOSE)
		std::cout << "VoxelSystem destroyed\n";
}
/// ---


/// Private functions

void	VoxelSystem::setCamera(Camera *cam)
{
	camera = cam;
}

void	VoxelSystem::updateDrawCommands()
{
	VDrawCommandMutex.lock();
	
	for (std::pair<glm::ivec3, DrawCommandData> cmds : cmdData) {
		if (cmds.second.status == DCS_INRENDER)
			continue ;

		for (size_t i = 0; i < 6; i++) {

			// TODO: update already generated chunks meshes next to the new one
			size_t	dataSize = cmds.second.vertices[i].size() * sizeof(DATA_TYPE);	

			if (!cmds.second.vertices[i].size())
				continue ;

			if (!VBO->write(cmds.second.vertices[i].data(), dataSize, cmds.second.cmd[i].baseInstance * sizeof(DATA_TYPE)))
				std::cout << "no more space\n";

			if (cmds.second.status == DCS_TOBEUPDATED) {
				commands[i] = cmds.second.cmd[i];
				chunksInfos[cmds.second.cmdIndex + i] = {{cmds.first.x, cmds.first.y, cmds.first.z, i}};
			}
			else {
				commands.push_back(cmds.second.cmd[i]);
				chunksInfos.push_back({{cmds.first.x, cmds.first.y, cmds.first.z, i}});
			}
		}
		cmdData[cmds.first].status = DCS_INRENDER;
	}
	//-cmdData.clear();
	VDrawCommandMutex.unlock();
}

// Update the indirect buffer
void	VoxelSystem::updateIB() {
	IB->bind();
	if (commands.size() * sizeof(DrawCommand) > IB->getCapacity()) {
		size_t newSize = commands.size() * sizeof(DrawCommand) * BUFFER_GROWTH_FACTOR;
		IB->resize(newSize, commands.data());
	}
	else
		IB->updateData(commands.data(), commands.size() * sizeof(DrawCommand), 0);
}

// Update the SSBO
void	VoxelSystem::updateSSBO() {
	SSBO->bind();
	if (chunksInfos.size() * sizeof(SSBOData) > SSBO->getCapacity()) {
		size_t newSize = chunksInfos.size() * sizeof(SSBOData) * BUFFER_GROWTH_FACTOR;
		SSBO->resize(newSize, chunksInfos.data());
	}
	else
		SSBO->updateData(chunksInfos.data(), chunksInfos.size() * sizeof(SSBOData), 0);
}

// Check if the voxel at the given position is visible
// Return a bitmask of the visible faces (6 bits : ZzYyXx)
uint8_t	VoxelSystem::isVoxelVisible(const glm::ivec3 &wPos, const ChunkData &data, AChunk *neightboursChunks[6], const size_t &LOD)
{
	size_t	x = wPos.x;
	size_t	y = wPos.y;
	size_t	z = wPos.z;
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
void	VoxelSystem::genMesh(const ChunkData &chunk, const size_t &LOD)
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
				uint8_t visibleFaces = isVoxelVisible({x, y, z}, chunk, neightboursChunks, LOD);

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
	
	//-currentVertexOffset = cmdData[chunk.first].cmd[0].baseInstance;
	if (cmdData.count(chunk.first))
		currentVertexOffset = cmdData[chunk.first].cmdIndex * (pow(CHUNK_SIZE, 3) * 6);
	else
		currentVertexOffset = cmdData.size() * (pow(CHUNK_SIZE, 3) * 6);

	for (int i = 0; i < 6; i++)
		datas.vertices[i] = vertices[i];
	for (int i = 0; i < 6; i++) {
		datas.cmd[i] = {4, (GLuint)vertices[i].size(), 0, (GLuint)currentVertexOffset};
		currentVertexOffset += vertices[i].size();
	}

	if (cmdData.count(chunk.first)) {
		if (cmdData[chunk.first].status == DCS_NEW)
			return ;
		datas.cmdIndex = cmdData[chunk.first].cmdIndex;
		cmdData[chunk.first] = datas;
		cmdData[chunk.first].status = DCS_TOBEUPDATED;
	}
	else {
		if (commands.size())
			datas.cmdIndex = commands.size() - 1;
		else 
			datas.cmdIndex = 0;
		cmdData[chunk.first] = datas;
		cmdData[chunk.first].status = DCS_NEW;
	}
}

void	VoxelSystem::chunkGenRoutine()
{
	std::list<glm::ivec3>	localReqChunks;
	ChunkMap				localChunks;

	while (42) {
		if (quitting)
			break ;

		size_t	chunkBatchLimit = 0;

		// check for new chunks to be generated
		if (requestedChunks.size()) {
			requestedChunkMutex.lock();
			for (glm::ivec3 rc : requestedChunks) {
				if (chunkBatchLimit == 1024)
					break ;
				localReqChunks.push_back(rc);
				chunkBatchLimit++;
			}
			for (auto k = requestedChunks.begin(); chunkBatchLimit && requestedChunks.size(); chunkBatchLimit--) {
				requestedChunks.erase(k);
				k = requestedChunks.begin();
			}
			requestedChunkMutex.unlock();
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
		pendingChunkMutex.lock();
		for (ChunkData chunk : localChunks) {
			chunks[chunk.first] = chunk.second;
			pendingChunks[chunk.first] = chunk.second;
		}
		pendingChunkMutex.unlock();
		localChunks.clear();
	}
}

void	VoxelSystem::meshGenRoutine()
{
	ChunkMap	localPendingChunks;
	static size_t	LOD = 4;
	
	while (42) {
		if (quitting)
			break ;

		// Check for chunks Mesh to be generated
		if (pendingChunks.size()) {
			pendingChunkMutex.lock();
			for (std::pair<glm::ivec3, AChunk *> chunk : pendingChunks)
				localPendingChunks[chunk.first] = chunk.second;

			pendingChunks.clear();
			pendingChunkMutex.unlock();
		}

		// Generate chunks meshes and creates their DrawCommands
		size_t	count = 0;

		if (VDrawCommandMutex.try_lock()) {
			for (std::pair<glm::ivec3, AChunk *> chunk : localPendingChunks) {
				glm::vec3	camPos = camera->getCameraInfo().position;
				glm::ivec3	camCpos = {camPos.z / 32, camPos.y / 32, camPos.x / 32};
				
				glm::ivec3	dst = {abs(camCpos.x - chunk.first.x), abs(camCpos.y - chunk.first.y), abs(camCpos.z - chunk.first.z)};

				if (dst.x >= 2 && dst.y >= 2 && dst.z >= 2)
					genMesh(chunk, LOD);
				else
					genMesh(chunk, 1);

				count++;
				//-std::cout << (int)cmdData[chunk.first].status << std::endl;
			}
			VDrawCommandMutex.unlock();
		}
		localPendingChunks.clear();

		// Check if any mesh has been created
		if (!count) {
			usleep(1000 * 100);
			continue ;
		}

		// Signal the main thread that it can update openGL's buffers
		updatingBuffers = true;
	}
}
/// ---


/// Public functions

void	VoxelSystem::requestChunk(const glm::ivec3 &start, const glm::ivec3 &end)
{
	if (!requestedChunkMutex.try_lock())
		return ;
	for (int i = start.x; i < end.x; i++)
		for (int j = start.y; j < end.y; j++)
			for (int k = start.z; k < end.z; k++)
				requestChunk({i, j, k}, true);
	requestedChunkMutex.unlock();
}

void	VoxelSystem::requestChunk(const glm::ivec3 &pos, const bool &batched)
{
	if (chunks[pos] || std::find(requestedChunks.begin(), requestedChunks.end(), pos) != requestedChunks.end())
		return ;

	if (!batched) {
		if (!requestedChunkMutex.try_lock())
			return ;
		requestedChunks.push_back(pos);
		requestedChunkMutex.unlock();
		return ;
	}
	//-std::cout << "requesting " << pos.x << " " <<  pos.y << " " << pos.z << std::endl;
	requestedChunks.push_back(pos);
}

void	VoxelSystem::requestMeshUpdate(const glm::ivec3 &start, const glm::ivec3 &end)
{
	if (!pendingChunkMutex.try_lock())
		return ;
	for (int i = start.x; i < end.x; i++)
		for (int j = start.y; j < end.y; j++)
			for (int k = start.z; k < end.z; k++)
				requestMeshUpdate({i, j, k}, true);
	pendingChunkMutex.unlock();
}

void	VoxelSystem::requestMeshUpdate(const glm::ivec3 &pos, const bool &batched)
{
	if (!cmdData.count(pos))
		return ;

	if (!batched) {
		if (!pendingChunkMutex.try_lock())
			return ;
		pendingChunks[pos] = chunks[pos];
		pendingChunkMutex.unlock();
		return ;
	}
	std::cout << "mesh " << pos.x << " " <<  pos.y << " " << pos.z << std::endl;
	pendingChunks[pos] = chunks[pos];
}

// Draw all chunks using batched rendering
GeoFrameBuffers	VoxelSystem::draw() {
	if (updatingBuffers) {
		updateDrawCommands();
		updateIB();
		updateSSBO();
		updatingBuffers = false;
	}

	// Binding the gBuffer
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// World DrawCall
	glBindVertexArray(VAO);
	VBO->bind();
	IB->bind();
	SSBO->bind();

	glBindTexture(GL_TEXTURE_2D, textures);

	glMultiDrawArraysIndirect(GL_TRIANGLE_STRIP, nullptr, commands.size(), sizeof(DrawCommand));

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	glBindVertexArray(0);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return (GeoFrameBuffers) {
		gBuffer,
		gPosition,
		gNormal,
		gColor
	};
}
/// ---
//// ----
