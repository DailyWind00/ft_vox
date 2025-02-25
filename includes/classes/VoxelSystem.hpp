#pragma once

/// Defines
# define GLM_ENABLE_EXPERIMENTAL
# define DATA_TYPE uint32_t
# define CHUNK_SIZE 32
# define HORIZONTAL_RENDER_DISTANCE 2
# define VERTICAL_RENDER_DISTANCE 1
# define BUFFER_GROWTH_FACTOR 1.5f
# define BATCH_LIMIT 100
# define THREAD_SLEEP_DURATION 10 // in ms
# define MIN_LOD (size_t)4
# define MAX_LOD (size_t)1

/// System includes
# include <iostream>
# include <algorithm>
# include <unordered_map>
# include <vector>
# include <thread>
# include <mutex>

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

typedef struct {
	AChunk *	chunk;
	size_t		LOD;
	ivec3		Wpos;
} ChunkData;
typedef unordered_map<ivec3, ChunkData> ChunkMap; // Wpos -> Chunk

// Class VoxelSystem
// This class is responsible for managing the voxel system 
// It have 2 child threads: ChunkGeneration & MeshGeneration
class VoxelSystem {
	private:
		ChunkMap	_chunks; // ChunkGeneration output
		Camera &	_camera;

		// OpenGL variables
		GLuint			_VAO;
		GLuint			_textureAtlas;
		GeoFrameBuffers	_gBuffer;

		// OpenGL Buffers (MeshGeneration output)
		PMapBufferGL *	_VBO;
		PMapBufferGL *	_IB;
		PMapBufferGL *	_SSBO;

		size_t	_VBO_Offset  = 0;
		size_t	_IB_Offset   = 0;
		size_t	_SSBO_Offset = 0;

		// Multi-threading
		thread	_chunkGenerationThread;
		thread	_meshGenerationThread;
		bool	_quitting = false;

		vector<ivec3>	_requestedChunks;
		vector<ivec3>	_requestedMeshes;

		mutex	_requestedChunksMutex;
		mutex	_requestedMeshesMutex;
		mutex	_chunksMutex;

		/// Private functions

		void	_chunkGenerationRoutine();
		void	_meshGenerationRoutine();

	public:
		VoxelSystem(const uint64_t &seed, Camera &camera); // seed 0 = random seed
		~VoxelSystem();

		/// Public functions

		void	requestChunk(const vector<ivec3> &Wpositions);
		void	requestMeshUpdate(const vector<ivec3> &Wpositions);

		const GeoFrameBuffers &	draw();

		/// Setters

		void	setCamera(Camera &cam);
};