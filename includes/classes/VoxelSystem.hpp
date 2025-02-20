#pragma once

/// Defines
# define GLM_ENABLE_EXPERIMENTAL
# define DATA_TYPE uint32_t
# define CHUNK_SIZE 32
# define HORIZONTALE_RENDER_DISTANCE 30
# define VERTICALE_RENDER_DISTANCE 12
# define BUFFER_GROWTH_FACTOR 1.5f

/// System includes
# include <iostream>
# include <unordered_map>
# include <vector>
# include <thread>
# include <mutex>
# include <atomic>

/// Dependencies
# include <glad/glad.h>
# include "glm/gtx/hash.hpp"
# include "BufferGL.hpp"
# include "PMapBufferGL.hpp"
# include "Noise.hpp"
# include "Camera.hpp"
# include "chunk.h"

/// Global variables
extern bool VERBOSE;

using namespace std;
using namespace glm;

// Data structure of a draw command
typedef struct {
	GLuint	verticeCount;
	GLuint	instanceCount;
	GLuint	offset;
 	GLuint	baseInstance;
} DrawCommand;

// Data structure for OpenGL buffers (VBO, IB, SSBO)
typedef struct {
	vector<AChunk*>			chunk;
	vector<DATA_TYPE>		vertices; // VBO
	vector<DrawCommand>		cmd;      // IB[6]
	vector<ivec4>			worldPos; // SSBO (x, y, z, LOD)
	vector<atomic<bool>>	isLocked;
} ChunkData;

// Data structure for the G-Buffer (Geometry pass)
typedef struct {
	GLuint	gBuffer;
	GLuint	gPosition;
	GLuint	gNormal;
	GLuint	gColor;
} GeoFrameBuffers;

// Class VoxelSystem
// This class is responsible for managing the voxel system
// It have 2 child threads: ChunkGeneration & MeshGeneration
class VoxelSystem {
	private:
		ChunkData	_chunks;
		Camera &	_camera;

		// OpenGL Buffers
		GLuint			_VAO;
		GLuint			_textureAtlas;

		BufferGL *		_IB;
		BufferGL *		_SSBO;
		PMapBufferGL *	_VBO;
		size_t			_VBOOffset = 0;

		GeoFrameBuffers	_gBuffer;

		// Multi-threading
		thread	_chunkGenerationThread;
		thread	_meshGenerationThread;
		bool	_quitting = false;

		/// Private functions

		void	_chunkGenerationRoutine();
		void	_meshGenerationRoutine();

		void	_updateIB();
		void	_updateSSBO();

	public:
		VoxelSystem(const uint64_t &seed, Camera &camera); // seed 0 = random seed
		~VoxelSystem();

		/// Public functions

		const GeoFrameBuffers &	draw();

		/// Setters

		void	setCamera(Camera &cam);
};