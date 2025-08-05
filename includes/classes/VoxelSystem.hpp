#pragma once

/// Defines
# define GLM_ENABLE_EXPERIMENTAL
# define DATA_TYPE uint32_t
# define CHUNK_SIZE 32
# define HORIZONTAL_RENDER_DISTANCE 64
# define VERTICAL_RENDER_DISTANCE 4
# define BUFFER_GROWTH_FACTOR 2
# define MESH_BATCH_LIMIT (size_t)2048
# define CHUNK_BATCH_LIMIT (size_t)128
# define THREAD_SLEEP_DURATION 10 // in ms
# define MIN_LOD (size_t)4
# define MAX_LOD (size_t)1
# define PLAYER_REACH 4 // in blocks

/// System includes
# include <iostream>
# include <algorithm>
# include <unordered_map>
# include <vector>
# include <deque>
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
typedef struct BufferArea {
    size_t offset;
    size_t size;
} BufferArea;

typedef struct ChunkData {
	AChunk *	chunk;
	ivec3		Wpos;
	size_t		LOD = 0;
	bool		neigthbourUpdated = false;
	bool		inCreation = true;

	deque<BufferArea>	VBO_area  = {{0, 0}};
	deque<BufferArea>	IB_area   = {{0, 0}};
	deque<BufferArea>	SSBO_area = {{0, 0}};

	inline bool hasMesh() const {
		return VBO_area.size() && VBO_area.back().size
			&& IB_area.size() && IB_area.back().size
			&& SSBO_area.size() && SSBO_area.back().size;
    }
} ChunkData;
typedef unordered_map<ivec3, ChunkData> ChunkMap; // Wpos -> ChunkData ptr

// Interface for chunk & mesh modifications
enum class ChunkAction {
	CREATE_UPDATE,
	DELETE
};
typedef pair<ivec3, ChunkAction> ChunkRequest; // Wpos, Action

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
		vector<ivec3>		_chunksToDelete;

		bool	_buffersNeedUpdates;
		mutex	_buffersMutex;

		// Multi-threading
		thread *	_chunkGenerationThreads;
		thread		_meshGenerationThread;
		bool		_quitting = false;
		uint32_t	_cpuCoreCount;

		deque<ChunkRequest>	_requestedChunks;
		deque<ChunkRequest>	_requestedMeshes;

		mutex	_requestedChunksMutex;
		mutex	_requestedMeshesMutex;
		mutex	_chunksMutex;

		/// Private functions

		void	_loadTextureAtlas();

		void	_chunkGenerationRoutine();
		void	_meshGenerationRoutine();

		void	_generateChunk(ChunkMap::value_type &chunk);
		void	_deleteChunk  (const ivec3 &pos);

		void	_generateMesh(ChunkData &chunk, ChunkData *neightboursChunks[6]);
		void	_deleteMesh  (ChunkData &chunk, ChunkData *neightboursChunks[6]);

		void	_updateBuffers();
		void	_writeInBuffer(PMapBufferGL *buffer, const void *data, const size_t &size, const size_t &offset);

	public:
		VoxelSystem(const uint64_t &seed, Camera &camera); // seed 0 = random seed
		~VoxelSystem();

		/// Public functions

		void	requestChunk(const vector<ChunkRequest> &requests);
		void	requestMesh (const vector<ChunkRequest> &requests);
		void	_chunkFloodFill(const glm::ivec3 &pos, const glm::ivec3 &oldPos, const ChunkAction &reqType, vector<ChunkRequest> *requests);

		void	tryDestroyBlock();
		const GeoFrameBuffers &	draw();

		/// Setters

		void	setCamera(Camera &cam);
};
