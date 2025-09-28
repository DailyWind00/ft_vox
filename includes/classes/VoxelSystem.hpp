#pragma once

/// Defines
# define GLM_ENABLE_EXPERIMENTAL
# define DATA_TYPE uint64_t
# define CHUNK_SIZE 32
# define HORIZONTAL_RENDER_DISTANCE 12
# define VERTICAL_RENDER_DISTANCE 6
# define SPAWN_LOCATION_SIZE	3
# define MESH_BATCH_LIMIT (size_t)2048
# define CHUNK_BATCH_LIMIT (size_t)128
# define THREAD_SLEEP_DURATION 10 // in ms
# define CHUNKGEN_CORE_RATIO	2
# define MIN_LOD (size_t)4
# define MAX_LOD (size_t)1
# define PLAYER_REACH 8 // in blocks

/// System includes
# include <iostream>
# include <algorithm>
# include <unordered_map>
# include <vector>
# include <deque>
# include <thread>
# include <mutex>

/// Dependencies
# include <glad/glad.h>
# include "glm/gtx/hash.hpp"
# include "Camera.hpp"
# include <Shader.hpp>
# include "chunk.h"

/// Global variables
extern bool VERBOSE;

using namespace std;
using namespace glm;

// Data structure for the G-Buffer (Geometry pass)
typedef struct GeoFrameBuffers {
	GLuint	gBuffer;
	GLuint	gPosition;
	GLuint	gNormal;
	GLuint	gColor;
} GeoFrameBuffers;

typedef struct MeshData {
	ChunkMesh *	mesh;
	bool		toBeDeleted = false;
} MeshData;
typedef unordered_map<glm::ivec3, MeshData>	MeshMap;

typedef struct ChunkData {
	AChunk *	chunk;
	glm::ivec3	Wpos;
	bool		neigthbourUpdated = false;
	bool		inCreation = true;
} ChunkData;
typedef unordered_map<glm::ivec3, ChunkData>	ChunkMap; // Wpos -> ChunkData ptr

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
		list<ChunkMesh *>	_meshesToDelete;
		MeshMap		_meshes; // MeshGeneration output
		ChunkMap	_chunks; // ChunkGeneration output
		Camera &	_camera;

		// OpenGL variables
		GLuint		_textureAtlas;
		GeoFrameBuffers	_gBuffer;

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
		mutex	_meshesMutex;
		mutex	_meshesToDeleteMutex;

		/// Private functions

		// Initialization functions
		void	_genWorldSpawn();
		void	_initThreads();
		void	_initDefferedRenderingPipeline();
		void	_loadTextureAtlas();

		// Thread routines
		void	_chunkGenerationRoutine();
		void	_meshGenerationRoutine();

		void	_generateChunk(ChunkMap::value_type &chunk);
		void	_deleteChunk  (const ivec3 &pos);

		ChunkMesh *	_generateMesh(ChunkData &chunk, ChunkData *neightboursChunks[6], const uint8_t &LOD, bool &deletePrevMesh);
		void	_constructChunkMesh(std::vector<DATA_TYPE> *vertices, ChunkData &chunk, ChunkData *neightboursChunks[6], const uint8_t &LOD);
		bool	_deleteMesh(ChunkData &chunk, ChunkData *neightboursChunks[6]);

	public:
		VoxelSystem(const uint64_t &seed, Camera &camera); // seed 0 = random seed
		~VoxelSystem();

		/// Public functions

		void	requestChunk(const list<ChunkRequest> &requests); // Todo : set private
		void	requestMesh (const list<ChunkRequest> &requests); // Todo : set private
		void	findChunksToDelete(list<ChunkRequest> &requestReturnList);

		void	tryDestroyBlock();
		const GeoFrameBuffers &	draw(ShaderHandler &shader);

		/// Setters

		void	setCamera(Camera &cam);

		/// Getters

		size_t	getChunkRequestCount();
		size_t	getMeshRequestCount();
};
