#pragma once

/// Defines
# define GLM_ENABLE_EXPERIMENTAL
# define DATA_TYPE uint32_t
# define CHUNK_SIZE 32
# define HORIZONTAL_RENDER_DISTANCE 5
# define VERTICAL_RENDER_DISTANCE 5
# define BUFFER_GROWTH_FACTOR 2
# define BATCH_LIMIT (size_t)100
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
typedef struct DrawCommand {
	GLuint	verticeCount  = 0;
	GLuint	instanceCount = 0;
	GLuint	offset        = 0;
 	GLuint	baseInstance  = 0;
} DrawCommand;

// Data structure for the SSBO (Shader Storage Buffer Object)
typedef struct SSBOData {
	ivec4	worldPos; // Wpos x y z, face orientation
} SSBOData;

// Data structure for the G-Buffer (Geometry pass)
typedef struct GeoFrameBuffers {
	GLuint	gBuffer;
	GLuint	gPosition;
	GLuint	gNormal;
	GLuint	gColor;
} GeoFrameBuffers;

// Data structure for CPU-side chunk data management
typedef struct ChunkData {
	// For creation
	AChunk *	chunk;
	size_t		LOD = 0;
	ivec3		Wpos;

	// For deletion
	size_t		VBO_area[2]  = {0, 0};   // offset, size
	size_t		IB_area[2]   = {0, 0};   // offset, size
	size_t		SSBO_area[2] = {0, 0};   // offset, size

} ChunkData;
typedef unordered_map<ivec3, ChunkData> ChunkMap; // Wpos -> ChunkData

// Interface for mesh modifications
enum class ChunkAction {
	NONE,
	CREATE_UPDATE,
	DELETE
};
typedef pair<ivec3, ChunkAction> MeshRequest; // Wpos, Action

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

		size_t	_VBO_size  = 0;
		size_t	_IB_size   = 0;
		size_t	_SSBO_size = 0;

		vector<DATA_TYPE>	_VBO_data;
		vector<DrawCommand>	_IB_data;
		vector<SSBOData>	_SSBO_data;
		vector<ChunkData>	_chunksToDelete;

		atomic<bool>	_buffersNeedUpdates;

		// Multi-threading
		thread	_chunkGenerationThread;
		thread	_meshGenerationThread;
		bool	_quitting = false;

		vector<ivec3>		_requestedChunks; // Wpos
		vector<MeshRequest>	_requestedMeshes; // Wpos, Action

		mutex	_requestedChunksMutex;
		mutex	_requestedMeshesMutex;
		mutex	_chunksMutex;

		/// Private functions

		void	_loadTextureAtlas();

		void	_chunkGenerationRoutine();
		void	_meshGenerationRoutine();

		void	_generateMesh(ChunkData &chunk, ChunkData *neightboursChunks[6]);
		void	_deleteChunk (ChunkData &chunk, ChunkData *neightboursChunks[6]);

		void	_writeInBuffer(PMapBufferGL *buffer, const void *data, const size_t &size, const size_t &offset);

	public:
		VoxelSystem(const uint64_t &seed, Camera &camera); // seed 0 = random seed
		~VoxelSystem();

		/// Public functions

		void	requestChunk(const vector<ivec3> &Wpositions);
		void	requestMeshUpdate(const vector<ivec3> &Wpositions, const ChunkAction &action);

		const GeoFrameBuffers &	draw();

		/// Setters

		void	setCamera(Camera &cam);
};