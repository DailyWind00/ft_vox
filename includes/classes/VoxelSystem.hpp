#pragma once
/// Defines
# define COLOR_HEADER_CXX
# define HORIZONTALE_RENDER_DISTANCE 30
# define VERTICALE_RENDER_DISTANCE 12
# define CHUNK_SIZE 32
# define DATA_TYPE uint32_t
# define BUFFER_GROWTH_FACTOR 1.5f
# define GLM_ENABLE_EXPERIMENTAL

/// System includes
# include <iostream>
# include <list>
# include <vector>
# include <mutex>
# include <thread>
# include <atomic>
# include <type_traits>
# include <unordered_map>

/// Dependencies
# include <glad/glad.h>
# include <glfw/glfw3.h>
# include "glm/gtx/hash.hpp" // Required for glm::ivec3 hash
# include "BufferGL.hpp"
# include "PMapBufferGL.hpp"
# include "Noise.hpp"
# include "color.h"
# include "Camera.hpp"
# include "chunk.h"

/// Global variables
extern bool VERBOSE;

// Data structure for CPU-side chunk data management
typedef std::pair<glm::ivec3, AChunk *>	ChunkData;
typedef std::unordered_map<glm::ivec3, AChunk *>	ChunkMap;

// Data structure for SSBO (Shader Storage Buffer Object)
typedef struct SSBOData {
	glm::ivec4	worldPos;
}	SSBOData;
typedef std::vector<SSBOData> VSSBOs;

// Data structure for IB (DrawArraysIndirectCommand)
typedef struct {
	GLuint	verticeCount;
	GLuint	instanceCount;
	GLuint	offset;
 	GLuint	baseInstance;
}	DrawCommand;
typedef std::vector<DrawCommand>	VDrawCommand;

typedef struct {
	std::vector<DATA_TYPE>	vertices[6];
	DrawCommand		cmd[6];
	uint32_t		cmdIndex;
	uint8_t			status;
}	DrawCommandData;
typedef std::unordered_map<glm::ivec3, DrawCommandData>	DrawCommandDataMap;

enum	drawCommandStates {
	DCS_INRENDER = 0,
	DCS_TOBEUPDATED,
	DCS_NEW
};

// Core class for the voxel system
// Create chunks and their meshes & manage their rendering
class	VoxelSystem {
	// This class use persistent mapped buffers (VBOs) with glMultiDrawArraysIndirect
	// This allow:
	//   - Load/update/delete chunks efficiently
	//   - Batch all chunks in a single draw call
	private:
		GLuint		VAO;
		ChunkMap	chunks;
		Camera *	camera;

		// Multithreading related data
		std::thread		meshGenThread;
		std::thread		chunkGenThread;

		std::list<glm::ivec3>	requestedChunks;
		std::mutex		requestedChunkMutex;

		ChunkMap		pendingChunks;
		std::mutex		pendingChunkMutex;

		DrawCommandDataMap	cmdData;
		std::mutex		VDrawCommandMutex;

		std::atomic<bool>	updatingBuffers;
		bool			quitting;

		// Deferred Rendering Buffers
		GLuint	gBuffer;
		GLuint	gPosition;
		GLuint	gNormal;
		GLuint	gColor;

		// Voxels Texture Atlas
		GLuint	textures;
		
		// Vertex Buffer Object
		PMapBufferGL *	VBO;
		size_t			currentVertexOffset = 0;

		// Indirect Buffer
		BufferGL *		IB;
		VDrawCommand	commands;

		// Shader Storage Buffer Object
		BufferGL *		SSBO;
		VSSBOs			chunksInfos;

		/// Private functions
		void	updateDrawCommands();
		void	updateIB();
		void	updateSSBO();
		uint8_t	isVoxelVisible(const glm::ivec3 &wPos, const ChunkData &data, AChunk *neightboursChunks[6], const size_t &LOD);

	
		void	chunkGenRoutine();
		void	meshGenRoutine();

		void	genMesh(const ChunkData &data, const size_t &LOD);	

	public:
		VoxelSystem(); // Random seed
		VoxelSystem(const uint64_t &seed); // Custom seed
		~VoxelSystem();

		/// Public functions
		void	setCamera(Camera *cam);

		void	requestChunk(const glm::ivec3 &pos, const bool &batched);
		void	requestChunk(const glm::ivec3 &start, const glm::ivec3 &end);

		void	requestMeshUpdate(const glm::ivec3 &pos, const bool &batched);
		void	requestMeshUpdate(const glm::ivec3 &start, const glm::ivec3 &end);
		
		struct GeoFrameBuffers	draw();
};
