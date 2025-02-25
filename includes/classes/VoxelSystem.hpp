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

// Data structure of a OpenGL draw command
// Used for indirect rendering
typedef struct {
	GLuint	verticeCount;
	GLuint	instanceCount;
	GLuint	offset;
 	GLuint	baseInstance;
} DrawCommand;

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
		vector<AChunk*>	_chunks; // ChunkGeneration output
		Camera &		_camera;

		// OpenGL variables
		GLuint			_VAO;
		GLuint			_textureAtlas;
		GeoFrameBuffers	_gBuffer;

		// OpenGL Buffers (MeshGeneration output)
		PMapBufferGL *	_VBO;
		PMapBufferGL *	_IB;
		PMapBufferGL *	_SSBO;

		size_t			_VBO_Offset  = 0;
		size_t			_IB_Offset   = 0;
		size_t			_SSBO_Offset = 0;

		// Multi-threading
		thread	_chunkGenerationThread;
		thread	_meshGenerationThread;
		bool	_quitting = false;

		/// Private functions

		void	_chunkGenerationRoutine();
		void	_meshGenerationRoutine();

	public:
		VoxelSystem(const uint64_t &seed, Camera &camera); // seed 0 = random seed
		~VoxelSystem();

		/// Public functions

		const GeoFrameBuffers &	draw();

		/// Setters

		void	setCamera(Camera &cam);
};