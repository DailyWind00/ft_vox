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

	size_t maxVerticesPerChunk = pow(CHUNK_SIZE, 3) * 6; // Worst case scenario
	size_t VBOcapacity = VERTICAL_RENDER_DISTANCE * pow(HORIZONTAL_RENDER_DISTANCE, 2) * maxVerticesPerChunk * sizeof(DATA_TYPE);

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
	size_t SSBOcapacity = VERTICAL_RENDER_DISTANCE * pow(HORIZONTAL_RENDER_DISTANCE, 2) * sizeof(SSBOData);

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
	_chunkGenerationThread = thread(&VoxelSystem::_chunkGenerationRoutine, this);
	_meshGenerationThread = thread(&VoxelSystem::_meshGenerationRoutine, this);

	// Request the chunks around the camera
	vector<ivec3>	positions;
	positions.reserve(pow(HORIZONTAL_RENDER_DISTANCE * 2 - 1, 2) * (VERTICAL_RENDER_DISTANCE * 2 - 1));

	for (int i = -HORIZONTAL_RENDER_DISTANCE + 1; i <= HORIZONTAL_RENDER_DISTANCE - 1; i++)
		for (int j = -VERTICAL_RENDER_DISTANCE + 1; j <= VERTICAL_RENDER_DISTANCE - 1; j++)
			for (int k = -HORIZONTAL_RENDER_DISTANCE + 1; k <= HORIZONTAL_RENDER_DISTANCE - 1; k++)
				positions.push_back(ivec3{i, j, k});

	requestChunk(positions);

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
	_chunkGenerationThread.join();
	_meshGenerationThread.join();

	// Delete all chunks
	for (auto &chunk : _chunks)
		delete chunk.second.chunk;

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
	stbi_set_flip_vertically_on_load(true);
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

	glGenerateMipmap(GL_TEXTURE_2D);

	// Set the texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	stbi_image_free(data);

	if (VERBOSE)
		cout << BGreen << "Done" << ResetColor << endl;
}

// Write data to the OpenGL buffers
void	VoxelSystem::_writeInBuffer(PMapBufferGL *buffer, const void *data, const size_t &size, const size_t &offset) {
	if (size + offset > buffer->getCapacity()) {
		buffer->resize(buffer->getCapacity() * BUFFER_GROWTH_FACTOR);

		// Update attributes
		if (buffer == _VBO) {
			glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(DATA_TYPE), nullptr);
			glVertexAttribDivisor(1, 1);	
			glEnableVertexAttribArray(1);
		}
		else if (buffer == _SSBO)
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _SSBO->getID());
	}

	buffer->write(data, size, offset);
	buffer->flush(offset, size);
}
/// ---



/// Public functions

// Draw all chunks using batched rendering
const GeoFrameBuffers	&VoxelSystem::draw() {
	static size_t	drawCount = 0;

	if (_buffersNeedUpdates) {
		drawCount += _IB_data.size();

		if (_VBO_data.size()) {
			_writeInBuffer(_VBO, _VBO_data.data(), _VBO_data.size() * sizeof(DATA_TYPE), _VBO_size);
			_VBO_size += _VBO_data.size() * sizeof(DATA_TYPE);
			_VBO_data.clear();
		}

		if (_IB_data.size()) {
			_writeInBuffer(_IB, _IB_data.data(), _IB_data.size() * sizeof(DrawCommand), _IB_size);
			_IB_size += _IB_data.size() * sizeof(DrawCommand);
			_IB_data.clear();
		}

		if (_SSBO_data.size()) {
			_writeInBuffer(_SSBO, _SSBO_data.data(), _SSBO_data.size() * sizeof(SSBOData), _SSBO_size);
			_SSBO_size += _SSBO_data.size() * sizeof(SSBOData);
			_SSBO_data.clear();
		}

		if (_chunksToDelete.size()) {
			for (ChunkData &chunk : _chunksToDelete) {
				_writeInBuffer(_VBO, nullptr, chunk.VBO_area[0], chunk.VBO_area[1]);
				_writeInBuffer(_IB, nullptr, chunk.IB_area[0], chunk.IB_area[1]);
				_writeInBuffer(_SSBO, nullptr, chunk.SSBO_area[0], chunk.SSBO_area[1]);
			}
			_chunksToDelete.clear();
			// cout << "Chunks deleted\n";
		}

		_buffersNeedUpdates = false;
	}


	// Bind the gBuffer
	glBindFramebuffer(GL_FRAMEBUFFER, _gBuffer.gBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Setup OpenGL drawing
	glBindVertexArray(_VAO);
	_VBO->bind();
	_IB->bind();
	_SSBO->bind();

	glBindTexture(GL_TEXTURE_2D, _textureAtlas);

	glMultiDrawArraysIndirect(GL_TRIANGLE_STRIP, nullptr, drawCount, sizeof(DrawCommand));

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	glBindVertexArray(0);
	
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