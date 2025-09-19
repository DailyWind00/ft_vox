#include "VoxelSystem.hpp"

/// Private functions

// Mesh Generation thread routine
void	VoxelSystem::_meshGenerationRoutine() {
	if (VERBOSE)
		cout << "> Mesh Generation thread started" << endl;

	while (!_quitting) {

		// Check if there are meshes to generate, sleep if not
		if (!_requestedMeshes.size() || !_requestedMeshesMutex.try_lock()) {
			this_thread::sleep_for(chrono::milliseconds(THREAD_SLEEP_DURATION));
			continue;
		}

		deque<ChunkRequest> localRequestedMeshes = _requestedMeshes;
		_requestedMeshesMutex.unlock();

		// Generate meshes up to the batch limit
		size_t batchCount = 0;

		_chunksMutex.lock(Priority::LOW);
		_meshToDeleteMutex.lock();

		for (ChunkRequest request : localRequestedMeshes) {
			if (batchCount >= MESH_BATCH_LIMIT)
				break;

			batchCount++;

			ivec3	Wpos = request.first;
			if (_chunks.find(Wpos) == _chunks.end())
				continue;

			// Calculate the LOD of the chunk (cause crashes for now)
			_chunks[Wpos].LOD = 1; // TODO: implement LOD

			ChunkData data = _chunks[Wpos];

			// Search for neightbouring chunks
			const ivec3	neightboursPos[6] = { 
				{Wpos.x - 1, Wpos.y, Wpos.z}, {Wpos.x + 1, Wpos.y, Wpos.z}, // x axis
				{Wpos.x, Wpos.y - 1, Wpos.z}, {Wpos.x, Wpos.y + 1, Wpos.z}, // y axis
				{Wpos.x, Wpos.y, Wpos.z - 1}, {Wpos.x, Wpos.y, Wpos.z + 1}  // z axis
			};

			ChunkData	*neightboursChunks[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

			for (size_t i = 0; i < 6; i++) {
				// Check if the chunk exist and have data to work with
				if (_chunks.find(neightboursPos[i]) != _chunks.end())
					neightboursChunks[i] = &_chunks[neightboursPos[i]];
			}

			ChunkMesh *	newMesh = nullptr;

			// Execute the requested action on the chunk mesh
			switch (request.second) {
				case ChunkAction::CREATE_UPDATE:
					_chunksMutex.unlock(Priority::LOW);
					newMesh = _generateMesh(data, neightboursChunks, data.LOD);
					_chunksMutex.lock(Priority::LOW);
					if (newMesh != nullptr)
						_chunks[data.Wpos].mesh = newMesh;
					break;

				case ChunkAction::DELETE:
					_deleteMesh(_chunks[Wpos], neightboursChunks);
					break;
			}

			data.inCreation = false;
		}

		_meshToDeleteMutex.unlock();
		_chunksMutex.unlock(Priority::LOW);


		// Remove the generated meshes from the requested list
		_requestedMeshesMutex.lock();
		_requestedMeshes.erase(_requestedMeshes.begin(), _requestedMeshes.begin() + batchCount);
		_requestedMeshesMutex.unlock();
	}

	if (VERBOSE)
		cout << "Mesh Generation thread stopped" << endl;
}

// Create/update the mesh of the given chunk
// The data will be stored in the main thread at the end of OpenGL buffers
ChunkMesh *	VoxelSystem::_generateMesh(ChunkData &chunk, ChunkData *neightboursChunks[6], const uint8_t &LOD) {
	// Check if the chunk already have a mesh (in case of update)
	if (chunk.mesh)
		_deleteMesh(chunk, neightboursChunks);

	chunk.neigthbourUpdated = false;

	// Check if the chunk completely empty
	if (!chunk.chunk || (IS_CHUNK_COMPRESSED(chunk.chunk) && !BLOCK_AT(chunk.chunk, 0, 0, 0)))
		return NULL;

	vector<DATA_TYPE>	vertices;
	size_t		maxDataSizePerChunk = (pow(CHUNK_SIZE, 3) / 2) * 6 * sizeof(DATA_TYPE);

	vertices.reserve(maxDataSizePerChunk);

	_constructChunkMesh(&vertices, chunk, neightboursChunks, LOD);

	return new ChunkMesh(vertices);
}

// Delete the first mesh
void	VoxelSystem::_deleteMesh(ChunkData &chunk, ChunkData *neightboursChunks[6]) {
	if (_chunks.find(chunk.Wpos) == _chunks.end() || !chunk.mesh)
		return;

	if (!chunk.mesh)
		return ;

	_meshToDelete.push_back(chunk.mesh);
	chunk.mesh = nullptr;

	if (chunk.neigthbourUpdated)
		return;

	// Request the update of neighbouring chunks
	list<ChunkRequest>	neightboursRequests;

	for (size_t i = 0; i < 6; i++) {
		if (neightboursChunks[i]) {
			neightboursChunks[i]->neigthbourUpdated = true;
			neightboursRequests.push_back({neightboursChunks[i]->Wpos, ChunkAction::CREATE_UPDATE});
		}
	}

	requestMesh(neightboursRequests);
}
/// ---



/// Public functions

// Request an action on a chunk mesh :
//  - CREATE_UPDATE will generate or update the chunk mesh
//  - DELETE will delete the mesh
void	VoxelSystem::requestMesh(const list<ChunkRequest> &requests) {
	if (requests.empty())
		return;

	_requestedMeshesMutex.lock();

	for (ChunkRequest req : requests) {
		// Check if the request already exists
		if (find(_requestedMeshes.begin(), _requestedMeshes.end(), req) == _requestedMeshes.end())
			_requestedMeshes.push_back(req);
	}

	_requestedMeshesMutex.unlock();
}
/// ---
