#include "VoxelSystem.hpp"
#include "config.hpp" // Get the window size

/// Constructors & Destructors
VoxelSystem::VoxelSystem(const uint64_t &seed, Camera &camera) : _camera(camera) {
	if (VERBOSE)
		cout << "Creating VoxelSystem\n";

	// Set the seed of the noise generator
	if (!seed) {
		srand(time(nullptr));
		Noise::setSeed(rand());
	}
	else
		Noise::setSeed(seed);

	// Load the texture atlas
	_textureAtlas = 0;
	_loadTextureAtlas();

	// Create the VAO
	glGenVertexArrays(1, &_VAO);
	glBindVertexArray(_VAO);

	// Default QuadVBO
	GLuint	quadVBO = 0;
	GLfloat	quadVert[] = {
	//  positions   textures
		0, 1, 0,    0, 0,
		0, 1, 1,    0, 1,
		1, 1, 0,    1, 0,
		1, 1, 1,    1, 1
	};

	glGenBuffers(1, &quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVert), quadVert, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Create and allocate the OpenGL buffers with persistent mapping
	GLenum buffersUsage = PERSISTENT_BUFFER_USAGE | GL_MAP_FLUSH_EXPLICIT_BIT;

	size_t	maxDataSizePerChunk = (pow(CHUNK_SIZE, 3) / 2) * 6 * sizeof(DATA_TYPE);
	size_t	chunkCount = pow(HORIZONTAL_RENDER_DISTANCE, 2) - ((pow(HORIZONTAL_RENDER_DISTANCE, 2) - 1) / 2);
	chunkCount *= VERTICAL_RENDER_DISTANCE;

	size_t VBOcapacity = chunkCount * maxDataSizePerChunk;

	if (VERBOSE) { cout << "> VBO  : "; }
	_VBO = new PMapBufferGL(GL_ARRAY_BUFFER, VBOcapacity, buffersUsage);

	glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(DATA_TYPE), nullptr);
	glVertexAttribDivisor(1, 1);	
	glEnableVertexAttribArray(1);

	// Create IB
	size_t IBcapacity = VERTICAL_RENDER_DISTANCE * pow(HORIZONTAL_RENDER_DISTANCE, 2) * 6 * sizeof(DrawCommand);

	if (VERBOSE) { cout << "> IB   : "; }
	_IB = new PMapBufferGL(GL_DRAW_INDIRECT_BUFFER, IBcapacity, buffersUsage);

	// Create SSBO
	size_t SSBOcapacity = VERTICAL_RENDER_DISTANCE * pow(HORIZONTAL_RENDER_DISTANCE, 2) * 6 * sizeof(SSBOData);

	if (VERBOSE) { cout << "> SSBO : "; }
	_SSBO = new PMapBufferGL(GL_SHADER_STORAGE_BUFFER, SSBOcapacity, buffersUsage);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _SSBO->getID());

	// Create the G-Buffer
	glGenFramebuffers(1, &_gBuffer.gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, _gBuffer.gBuffer);

	// Position color buffer
	glGenTextures(1, &_gBuffer.gPosition);
	glBindTexture(GL_TEXTURE_2D, _gBuffer.gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _gBuffer.gPosition, 0);

	// Normal color buffer
	glGenTextures(1, &_gBuffer.gNormal);
	glBindTexture(GL_TEXTURE_2D, _gBuffer.gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _gBuffer.gNormal, 0);

	// Albedo color Buffer
	glGenTextures(1, &_gBuffer.gColor);
	glBindTexture(GL_TEXTURE_2D, _gBuffer.gColor);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, _gBuffer.gColor, 0);

	// Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
	GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,  GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	// Depth buffer
	GLuint rboDepth;

	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WINDOW_WIDTH, WINDOW_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Create the threads
	_cpuCoreCount = std::thread::hardware_concurrency();

	if (VERBOSE)
		cout << "System has: " << _cpuCoreCount << " CPU cores available.\n Allocating: " << (_cpuCoreCount / 1.5) - 1 << " for chunk generation" << endl;

	_chunkGenerationThreads = new thread[_cpuCoreCount / 4];
	for (uint32_t i = 0; i < _cpuCoreCount / 4; i++)
		_chunkGenerationThreads[i] = thread(&VoxelSystem::_chunkGenerationRoutine, this);
	_meshGenerationThread = thread(&VoxelSystem::_meshGenerationRoutine, this);

	// Request the chunks around the camera
	vector<ChunkRequest>	spawnChunks;
	spawnChunks.reserve(pow(HORIZONTAL_RENDER_DISTANCE * 2 - 1, 2) * (VERTICAL_RENDER_DISTANCE * 2 - 1));
	for (int i = 0; i < VERTICAL_RENDER_DISTANCE; i++) {
		_chunkFloodFill({0, i, 0}, {0, i, 0},  ChunkAction::CREATE_UPDATE, &spawnChunks);
		_chunkFloodFill({0, -i, 0}, {0, -i, 0},  ChunkAction::CREATE_UPDATE, &spawnChunks);
	}
	requestChunk(spawnChunks);

	if (VERBOSE)
		cout << "VoxelSystem initialized\n";
}

VoxelSystem::~VoxelSystem() {
	delete _VBO;
	delete _IB;
	delete _SSBO;
	glDeleteVertexArrays(1, &_VAO);

	glDeleteTextures(1, &_gBuffer.gPosition);
	glDeleteTextures(1, &_gBuffer.gNormal);
	glDeleteTextures(1, &_gBuffer.gColor);
	glDeleteFramebuffers(1, &_gBuffer.gBuffer);

	if (_textureAtlas)
		glDeleteTextures(1, &_textureAtlas);

	// waiting for threads to finish
	_quitting = true;
	for (uint32_t i = 0; i < _cpuCoreCount / 4; i++)
		_chunkGenerationThreads[i].join();
	_meshGenerationThread.join();

	// Delete all chunks
	for (const ChunkMap::value_type &chunk : _chunks) {
		if (chunk.second.chunk)
			delete chunk.second.chunk;
	}

	_chunks.clear();
	
	if (VERBOSE)
		cout << "VoxelSystem destroyed\n";
}
/// ---



/// Private functions

// Load/reload the texture atlas
void	VoxelSystem::_loadTextureAtlas() {
	if (VERBOSE)
		cout << "> Loading texture atlas -> ";

	// Load the texture atlas
	//-stbi_set_flip_vertically_on_load(true);
	int width, height, nrChannels;
	unsigned char *data = stbi_load("./assets/atlas.png", &width, &height, &nrChannels, 0);

	if (!data || !(nrChannels >= 3 && nrChannels <= 4)) {
		if (VERBOSE)
			cout << BRed << "Failed" << ResetColor << endl;

		return;
	}

	if (_textureAtlas)
		glDeleteTextures(1, &_textureAtlas);

	// Create the texture
	glGenTextures(1, &_textureAtlas);
	glBindTexture(GL_TEXTURE_2D, _textureAtlas);

	if (nrChannels == 3)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	else if (nrChannels == 4)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	// Set the texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(data);

	if (VERBOSE)
		cout << BGreen << "Done" << ResetColor << endl;
}

// Write data to the OpenGL buffers
void	VoxelSystem::_writeInBuffer(PMapBufferGL *buffer, const void *data, const size_t &size, const size_t &offset) {
	bool	resized = false;

	// Resize the buffer if needed
	if (size + offset > buffer->getCapacity()) {
		if (!buffer->resize(buffer->getCapacity() * BUFFER_GROWTH_FACTOR))
			throw runtime_error("Failed to resize PMapBufferGL");

		// Update attributes
		if (buffer == _VBO) {
			glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(DATA_TYPE), nullptr);
			glVertexAttribDivisor(1, 1);	
			glEnableVertexAttribArray(1);
		}
		else if (buffer == _SSBO)
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _SSBO->getID());

		resized = true;
	}

	buffer->write(data, size, offset);

	// Flush the whole buffer up to the new size if resized
	if (resized)
		buffer->flush(0, offset + size);
	else
		buffer->flush(offset, size);

	if (buffer == _SSBO)
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}


// Update buffers if possible
void	VoxelSystem::_updateBuffers() {
	if (!_buffersMutex.try_lock())
		return;

	if (_buffersNeedUpdates) { // TODO : Add a limit to the number of updates

		// Delete the chunks (may cause lag so we batch it)
		if (_chunksToDelete.size() /* >= BATCH_LIMIT */) {
			static size_t	deletedCount = 0;

			for (const ivec3 &WPos : _chunksToDelete) {
				ChunkData &chunk = _chunks.find(WPos)->second;
				BufferArea &IB_area   = chunk.IB_area.back();
				BufferArea &SSBO_area = chunk.SSBO_area.back();

				// We loose pointer to the chunk data so VBO cleanup is not needed
				_writeInBuffer(_IB, nullptr, IB_area.size, IB_area.offset);
				_writeInBuffer(_SSBO, nullptr, SSBO_area.size, SSBO_area.offset);

				// Reset the chunk data
				if (!chunk.hasMesh()) {
					chunk.VBO_area.clear();
					chunk.IB_area.clear();
					chunk.SSBO_area.clear();
				}

				// Remove previous areas
 				while (chunk.VBO_area.size() > 1)
				{
					_writeInBuffer(_VBO, nullptr, chunk.VBO_area.front().size, chunk.VBO_area.front().offset);
					chunk.VBO_area.pop_front();
				}
				while (chunk.IB_area.size() > 1)
				{
					_writeInBuffer(_IB, nullptr, chunk.IB_area.front().size, chunk.IB_area.front().offset);
					chunk.IB_area.pop_front();
				}
				while (chunk.SSBO_area.size() > 1)
				{
					_writeInBuffer(_SSBO, nullptr, chunk.SSBO_area.front().size, chunk.SSBO_area.front().offset);
					chunk.SSBO_area.pop_front();
				}

				// Delete the chunk from the map if asked by ChunkGeneration
				if (!chunk.chunk)
					_chunks.erase(chunk.Wpos);

				deletedCount++;
			}
			_chunksToDelete.clear();

			// TODO : Recompact the buffers if deletedCount > threshold
		}

		// VBO
		if (_VBO_data.size()) {
			_writeInBuffer(_VBO, _VBO_data.data(), _VBO_data.size() * sizeof(DATA_TYPE), _VBO_size);
			_VBO_size += _VBO_data.size() * sizeof(DATA_TYPE);
			_VBO_data.clear();
		}

		// IB
		if (_IB_data.size()) {
			_writeInBuffer(_IB, _IB_data.data(), _IB_data.size() * sizeof(DrawCommand), _IB_size);
			_IB_size += _IB_data.size() * sizeof(DrawCommand);
			_IB_data.clear();
		}

		// SSBO	
		if (_SSBO_data.size()) {
			_writeInBuffer(_SSBO, _SSBO_data.data(), _SSBO_data.size() * sizeof(SSBOData), _SSBO_size);
			_SSBO_size += _SSBO_data.size() * sizeof(SSBOData);
			_SSBO_data.clear();
		}

		_buffersNeedUpdates = false;
	}
	_buffersMutex.unlock();
}

static inline vec3 toVoxelCoords(const vec3 &vector) {
	return vec3 (
		vector.z,
		vector.y,
		vector.x
	);
}
/// ---



/// Public functions

/// @brief Try to destroy a block on where the currently set camera is looking at.
/// @details Raycast a ray from the camera position to the lookAt position, until it hits a block or PLAYER_REACH is reached.
void VoxelSystem::tryDestroyBlock()
{
	const CameraInfo &camInfo = _camera.getCameraInfo();
	vec3 worldCamPos = toVoxelCoords(camInfo.position);
	vec3 currentPos = toVoxelCoords(camInfo.position);
	vec3 lookAt = toVoxelCoords(camInfo.lookAt);
	
	do
	{
		// Get the chunk at the current position
		ivec3 chunkPos = {
			floor(currentPos.x / (float)CHUNK_SIZE),
			floor(currentPos.y / (float)CHUNK_SIZE),
			floor(currentPos.z / (float)CHUNK_SIZE)
		};
		ChunkMap::iterator it = _chunks.find(chunkPos);
		if (it == _chunks.end())
			return ;
		ChunkData &chunkData = it->second;
		if (!chunkData.chunk || !chunkData.hasMesh())
			return ;

		// Get the position of the current block in the chunk
		ivec3 localPos = {
			(int)mod(currentPos.z, (float)CHUNK_SIZE),
			(int)mod(currentPos.y, (float)CHUNK_SIZE),
			(int)mod(currentPos.x, (float)CHUNK_SIZE)
		};

		// Check if there is a block at the current position
		uint8_t blockID = BLOCK_AT(chunkData.chunk, localPos.x, localPos.y, localPos.z);
		if (blockID) {
			ChunkHandler::setBlock(chunkData.chunk, localPos, 0);
			requestMesh({{chunkPos, ChunkAction::CREATE_UPDATE}});

			if (VERBOSE)
				cout << BGreen << "Block destroyed at " << (int)currentPos.x << ", " << (int)currentPos.y << ", " << (int)currentPos.z << ResetColor << endl;

			return ;
		}

		// Move to the next position in the direction of the lookAt vector
		currentPos -= glm::normalize(worldCamPos - lookAt) * 0.1f;

		cout << "Current distance: " << distance(currentPos, worldCamPos) << endl;
		cout << "Current position: " << currentPos.x << ", " << currentPos.y << ", " << currentPos.z << endl;
	}
	while (distance(currentPos, worldCamPos) < PLAYER_REACH);

	if (VERBOSE)
		cout << BRed << "No block found" << ResetColor << endl;
}

// Draw all chunks using batched rendering
const GeoFrameBuffers	&VoxelSystem::draw() {
	_updateBuffers();

	// Bind the gBuffer
	glBindFramebuffer(GL_FRAMEBUFFER, _gBuffer.gBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Setup OpenGL drawing
	glBindVertexArray(_VAO);
	_VBO->bind();
	_IB->bind();
	_SSBO->bind();

	// Setup textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _textureAtlas);

	glMultiDrawArraysIndirect(GL_TRIANGLE_STRIP, nullptr, _IB_size / sizeof(DrawCommand), sizeof(DrawCommand));
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return _gBuffer;
}
/// ---



/// Setters

// Set the camera
void	VoxelSystem::setCamera(Camera &camera) {
	_camera = camera;
}
/// ---
