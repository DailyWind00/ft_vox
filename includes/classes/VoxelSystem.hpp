#pragma once
/// Defines
# define COLOR_HEADER_CXX
# define HORIZONTALE_RENDER_DISTANCE 48
# define VERTICALE_RENDER_DISTANCE 4
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
	glm::ivec3		wPos;
}	DrawCommandData;
typedef std::list<DrawCommandData>	VDrawCommandData;

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

		// Multithreading related data
		std::thread		meshGenThread;
		std::thread		chunkGenThread;

		std::list<glm::ivec3>	requestedChunks;
		std::mutex		requestedChunkMutex;

		ChunkMap		pendingChunks;
		std::mutex		pendingChunkMutex;

		VDrawCommandData	cmdData;
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
	
		void	chunkGenRoutine();

		void			meshGenRoutine();
		DrawCommandData	genMesh(const ChunkData &data);
		uint8_t			isVoxelVisible(const size_t &x, const size_t &y, const size_t &z,
										const ChunkData &data, AChunk *neightboursChunks[6]);

	public:
		VoxelSystem(); // Random seed
		VoxelSystem(const uint64_t &seed); // Custom seed
		~VoxelSystem();

		/// Public functions
		struct GeoFrameBuffers	draw();
};
