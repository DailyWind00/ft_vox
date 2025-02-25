#include "VoxelSystem.hpp"
#include "config.hpp"

/// Constructors & Destructors
VoxelSystem::VoxelSystem(const uint64_t &seed, Camera &camera) : _camera(camera) {
	if (VERBOSE)
		cout << "Creating VoxelSystem\n";

	if (!seed) {
		srand(time(nullptr));
		Noise::setSeed(rand());
	}
	else
		Noise::setSeed(seed);

	// Create the VAO
	glGenVertexArrays(1, &_VAO);
	glBindVertexArray(_VAO);

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

	// Create and allocate the OpenGL buffers with persistent mapping
	GLenum buffersUsage = PERSISTENT_BUFFER_USAGE + GL_MAP_FLUSH_EXPLICIT_BIT;

	size_t maxVerticesPerChunk = (pow(CHUNK_SIZE, 3) / 2 + (CHUNK_SIZE % 2)) * 6; // Worst case scenario
	size_t VBOcapacity = VERTICALE_RENDER_DISTANCE * pow(HORIZONTALE_RENDER_DISTANCE, 2) * maxVerticesPerChunk * sizeof(DATA_TYPE);

	if (VERBOSE) { cout << "> VBO  : "; }
	_VBO = new PMapBufferGL(GL_ARRAY_BUFFER, VBOcapacity, buffersUsage);

	glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(DATA_TYPE), nullptr);
	glVertexAttribDivisor(1, 1);	
	glEnableVertexAttribArray(1);

	// Create IB
	size_t IBcapacity = VERTICALE_RENDER_DISTANCE * pow(HORIZONTALE_RENDER_DISTANCE, 2) * 6 * sizeof(DrawCommand);

	if (VERBOSE) { cout << "> IB   : "; }
	_IB = new PMapBufferGL(GL_DRAW_INDIRECT_BUFFER, IBcapacity, buffersUsage);

	// Create SSBO
	size_t SSBOcapacity = VERTICALE_RENDER_DISTANCE * pow(HORIZONTALE_RENDER_DISTANCE, 2) * 6 * sizeof(ivec4);

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
	// _chunkGenerationThread = thread(&VoxelSystem::_chunkGenerationRoutine, this);
	// _meshGenerationThread = thread(&VoxelSystem::_meshGenerationRoutine, this);

	if (VERBOSE)
		cout << "VoxelSystem initialized\n";
}

VoxelSystem::~VoxelSystem() {
	delete _VBO;
	delete _IB;
	glDeleteVertexArrays(1, &_VAO);

	// waiting for threads to finish
	_quitting = true;
	// _chunkGenerationThread.join();
	// _meshGenerationThread.join();

	// Delete all chunks
	for (auto &chunk : _chunks)
		delete chunk;
	_chunks.clear();
	
	if (VERBOSE)
		cout << "VoxelSystem destroyed\n";
}
/// ---



/// Public functions

// Draw all chunks using batched rendering
const GeoFrameBuffers	&VoxelSystem::draw() {
	// if (updatingBuffers) {
	// 	updateDrawCommands();
	// 	updateIB();
	// 	updateSSBO();
	// 	updatingBuffers = false;
	// }

	// Binding the gBuffer
	glBindFramebuffer(GL_FRAMEBUFFER, _gBuffer.gBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// World DrawCall
	glBindVertexArray(_VAO);
	_VBO->bind();
	_IB->bind();
	_SSBO->bind();

	glBindTexture(GL_TEXTURE_2D, _textureAtlas);

	glMultiDrawArraysIndirect(GL_TRIANGLE_STRIP, nullptr, _chunks.size() * 6, sizeof(DrawCommand));

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