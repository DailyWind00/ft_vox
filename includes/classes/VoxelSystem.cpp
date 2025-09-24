#include "VoxelSystem.hpp"
#include "config.hpp" // Get the window size

# include <Shader.hpp>

/// Constructors & Destructors
VoxelSystem::VoxelSystem(const uint64_t &seed, Camera &camera, Camera &shadowMapCam) : _camera(camera), _shadowMapCam(shadowMapCam) {
	if (VERBOSE)
		cout << "Creating VoxelSystem\n";

	// Set the seed of the noise generator
	if (!seed) {
		srand(time(nullptr));

		int	rSeed = rand();

		std::cout << "Loading ft_vox with the random seed: " << rSeed << std::endl;

		Noise::setSeed(rSeed);
	}
	else
		Noise::setSeed(seed);

	// Load the texture atlas
	_textureAtlas = 0;
	_loadTextureAtlas();

	// Initialize the rendering pipeline
	_initDefferedRenderingPipeline();

	_initPostProcessingComponents();

	// Initialize the ShadowMapping Pipeline
	_initShadowMappingPipeline();

	// Initialize the threads
	_initThreads();

	// Generate chunks around the spawn location
	_genWorldSpawn();

	if (VERBOSE)
		cout << "VoxelSystem initialized\n";
}

VoxelSystem::~VoxelSystem() {
	// delete _VBO;
	// delete _IB;
	// delete _SSBO;
	// glDeleteVertexArrays(1, &_VAO);

	glDeleteTextures(1, &_gBuffer.gPosition);
	glDeleteTextures(1, &_gBuffer.gNormal);
	glDeleteTextures(1, &_gBuffer.gColor);
	glDeleteFramebuffers(1, &_gBuffer.gBuffer);

	if (_textureAtlas)
		glDeleteTextures(1, &_textureAtlas);

	// waiting for threads to finish
	_quitting = true;
	for (uint32_t i = 0; i < _cpuCoreCount / CHUNKGEN_CORE_RATIO; i++)
		_chunkGenerationThreads[i].join();
	_meshGenerationThread.join();

	// Delete all chunks
	for (const ChunkMap::value_type &chunk : _chunks)
		if (chunk.second.chunk)
			delete chunk.second.chunk;

	_chunks.clear();
	g_pendingFeatures.clear();

	if (VERBOSE)
		cout << "VoxelSystem destroyed\n";
}
/// ---



/// Private functions

// Generate chunks around the spawn location in a 3 or less chunk radius
void	VoxelSystem::_genWorldSpawn() {
	vector<ChunkRequest>	spawnChunks;
	const int		horizontalSpawnSize = glm::min(SPAWN_LOCATION_SIZE, HORIZONTAL_RENDER_DISTANCE);

	for (int i = -VERTICAL_RENDER_DISTANCE; i <= VERTICAL_RENDER_DISTANCE; i++)
		for (int j = -horizontalSpawnSize; j <= horizontalSpawnSize; j++)
			for (int k = -horizontalSpawnSize; k <= horizontalSpawnSize; k++)
				spawnChunks.push_back({{k, i, j}, ChunkAction::CREATE_UPDATE});
	requestChunk(spawnChunks);
}

// Will initialize and start all the multi-threading related systems
void	VoxelSystem::_initThreads() {
	// Get the CPU core count of the system
	_cpuCoreCount = std::thread::hardware_concurrency();

	if (VERBOSE)
		cout << "System has: " << _cpuCoreCount << " CPU cores available.\n Allocating: " << _cpuCoreCount / CHUNKGEN_CORE_RATIO << " for chunk generation" << endl;

	// Allocate and start chunk generation threads
	_chunkGenerationThreads = new thread[_cpuCoreCount / CHUNKGEN_CORE_RATIO];
	for (uint32_t i = 0; i < _cpuCoreCount / CHUNKGEN_CORE_RATIO; i++)
		_chunkGenerationThreads[i] = thread(&VoxelSystem::_chunkGenerationRoutine, this);

	// Start mesh generation thread
	_meshGenerationThread = thread(&VoxelSystem::_meshGenerationRoutine, this);
}

// Will create and setup all the framebuffer and render texture necessary for rendering
void	VoxelSystem::_initDefferedRenderingPipeline() {
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

	// Albedo color Buffer
	glGenTextures(1, &_gBuffer.gEmissive);
	glBindTexture(GL_TEXTURE_2D, _gBuffer.gEmissive);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, _gBuffer.gEmissive, 0);

	// Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
	GLuint attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,  GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, attachments);

	// Depth buffer
	GLuint rboDepth;

	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WINDOW_WIDTH, WINDOW_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void	VoxelSystem::_initPostProcessingComponents() {
	// Frame Buffer creation
	glGenFramebuffers(1, &_postProcData.postProcFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, _postProcData.postProcFBO);

	// Texture Buffer creation
	glGenTextures(1, &_postProcData.postProcBuffer);
	glBindTexture(GL_TEXTURE_2D, _postProcData.postProcBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _postProcData.postProcBuffer, 0);

	GLuint	attachements[1] {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, attachements);


	glGenTextures(1, &_postProcData.postProcDepthBuffer);
	glBindTexture(GL_TEXTURE_2D, _postProcData.postProcDepthBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _postProcData.postProcDepthBuffer, 0);
	

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void	VoxelSystem::_initShadowMappingPipeline() {
	// Testing shadow mapping
	glGenFramebuffers(1, &_shadowMapData.depthMapFBO);

	glGenTextures(1, &_shadowMapData.depthMap);
	glBindTexture(GL_TEXTURE_2D, _shadowMapData.depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_RESOLUTION, SHADOW_RESOLUTION, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float	borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, _shadowMapData.depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _shadowMapData.depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

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
		if (!chunkData.chunk || !chunkData.mesh)
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
	}
	while (distance(currentPos, worldCamPos) < PLAYER_REACH);

	if (VERBOSE)
		cout << BRed << "No block found" << ResetColor << endl;
}

uint8_t	VoxelSystem::getBlockAt(const glm::ivec3 &pos) {
	ivec3 chunkPos = {
		floor((float)pos.x / (float)CHUNK_SIZE),
		floor((float)pos.y / (float)CHUNK_SIZE),
		floor((float)pos.z / (float)CHUNK_SIZE)
	};

	ChunkMap::iterator it = _chunks.find(chunkPos);
	if (it == _chunks.end())
		return 0;

	ivec3 localPos = {
		mod((float)pos.z, (float)CHUNK_SIZE),
		mod((float)pos.y, (float)CHUNK_SIZE),
		mod((float)pos.x, (float)CHUNK_SIZE)
	};
	return BLOCK_AT(_chunks[chunkPos].chunk, localPos.x, localPos.y, localPos.z);
}

static inline const vec4	extractPlane(const mat4& m, int row, int sign) {
	vec4	plane = {
		m[0][3] + sign * m[0][row],
		m[1][3] + sign * m[1][row],
		m[2][3] + sign * m[2][row],
		m[3][3] + sign * m[3][row]
	};
	
	float	len = length(vec3(plane)); // normalize by xyz length
	return plane / len;
}

// Draw all chunks
const GeoFrameBuffers	&VoxelSystem::renderGeometryPass(ShaderHandler &shader) {
	// Bind the gBuffer
	glBindFramebuffer(GL_FRAMEBUFFER, _gBuffer.gBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Setup textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _textureAtlas);

	// Setup frustum culling
	mat4 VP = _camera; // Get the View-Projection matrix

	array<vec4, 6> frustumPlanes;
	frustumPlanes[0] = extractPlane(VP, 0, +1); // Left
	frustumPlanes[1] = extractPlane(VP, 0, -1); // Right
	frustumPlanes[2] = extractPlane(VP, 1, +1); // Bottom
	frustumPlanes[3] = extractPlane(VP, 1, -1); // Top
	frustumPlanes[4] = extractPlane(VP, 2, +1); // Near
	frustumPlanes[5] = extractPlane(VP, 2, -1); // Far

	for (ChunkMap::iterator it = _chunks.begin(); it != _chunks.end(); it++) {
		if (!it->second.mesh)
			continue ;

		// Frustum culling
		vec3 chunkCenter = vec3(it->first * CHUNK_SIZE + CHUNK_SIZE / 2);
		float chunkRadius = CHUNK_SIZE * sqrt(3) / 2.0f;

		bool inFrustrum = true;
		for (auto& plane : frustumPlanes) {
			float distance = dot(vec3(plane), chunkCenter) + plane.w;
			if (distance < -chunkRadius) { // sphere outside
				inFrustrum = false;
				break;
			}
		}
		if (!inFrustrum) continue;

		// Draw the chunk
		if (!it->second.mesh->getVAO())
			continue ;

		vec3	wPos = it->second.Wpos;
		shader.setUniform((*shader[1])->getID(), "worldPos", wPos);

		it->second.mesh->draw();
	}

	glDisable(GL_CULL_FACE);
	for (ChunkMap::iterator it = _chunks.begin(); it != _chunks.end(); it++) {
		if (!it->second.mesh)
			continue ;

		// Frustum culling
		vec3 chunkCenter = vec3(it->first * CHUNK_SIZE + CHUNK_SIZE / 2);
		float chunkRadius = CHUNK_SIZE * sqrt(3) / 2.0f;

		bool inFrustrum = true;
		for (auto& plane : frustumPlanes) {
			float distance = dot(vec3(plane), chunkCenter) + plane.w;
			if (distance < -chunkRadius) { // sphere outside
				inFrustrum = false;
				break;
			}
		}
		if (!inFrustrum) continue;

		// Draw the chunk
		if (!it->second.mesh->getWaterVAO())
			continue ;

		vec3	wPos = it->second.Wpos;
		shader.setUniform((*shader[1])->getID(), "worldPos", wPos);

		it->second.mesh->drawWater();
	}
	glEnable(GL_CULL_FACE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return _gBuffer;
}

// Draw all chunks
const ShadowMappingData	&VoxelSystem::renderShadowMapPass(ShaderHandler &shader) {
	// Bind the gBuffer
	glBindFramebuffer(GL_FRAMEBUFFER, _shadowMapData.depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	// Setup frustum culling
	mat4 VP = _shadowMapCam; // Get the View-Projection matrix

	array<vec4, 6> frustumPlanes;
	frustumPlanes[0] = extractPlane(VP, 0, +1); // Left
	frustumPlanes[1] = extractPlane(VP, 0, -1); // Right
	frustumPlanes[2] = extractPlane(VP, 1, +1); // Bottom
	frustumPlanes[3] = extractPlane(VP, 1, -1); // Top
	frustumPlanes[4] = extractPlane(VP, 2, +1); // Near
	frustumPlanes[5] = extractPlane(VP, 2, -1); // Far

	for (ChunkMap::iterator it = _chunks.begin(); it != _chunks.end(); it++) {
		if (!it->second.mesh)
			continue ;

		// Draw the chunk
		if (!it->second.mesh->getVAO() || !it->second.mesh->getWaterVAO())
			it->second.mesh->updateMesh();

		// Frustum culling
		vec3 chunkCenter = vec3(it->first * CHUNK_SIZE + CHUNK_SIZE / 2);
		float chunkRadius = CHUNK_SIZE * sqrt(3) / 2.0f;

		bool inFrustrum = true;
		for (auto& plane : frustumPlanes) {
			float distance = dot(vec3(plane), chunkCenter) + plane.w;
			if (distance < -chunkRadius) { // sphere outside
				inFrustrum = false;
				break;
			}
		}
		if (!inFrustrum) continue;


		vec3	wPos = it->second.Wpos;
		shader.setUniform((*shader[3])->getID(), "worldPos", wPos);

		it->second.mesh->draw();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return _shadowMapData;
}
/// ---



/// Setters

// Set the camera
void	VoxelSystem::setCamera(Camera &camera) {
	_camera = camera;
}

void	VoxelSystem::setShadowMapCam(Camera &camera) {
	_shadowMapCam = camera;
}
/// ---



/// Getters

const PostProcessingData &	VoxelSystem::getPostProcData() {
	return _postProcData;
}

size_t	VoxelSystem::getChunkRequestCount()
{
	return _requestedChunks.size();
}

size_t	VoxelSystem::getMeshRequestCount()
{
	return _requestedMeshes.size();
}

/// ---
