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

		while (_chunksMutex.try_lock() == false)
			this_thread::sleep_for(chrono::milliseconds(THREAD_SLEEP_DURATION));

		MeshMap	localMeshes;
		list<glm::ivec3>	localDeletion;

		for (ChunkRequest request : localRequestedMeshes) {
			if (batchCount >= MESH_BATCH_LIMIT)
				break;

			batchCount++;

			ivec3	Wpos = request.first;
			if (_chunks.find(Wpos) == _chunks.end())
				continue;

			ChunkData &data = _chunks[Wpos];

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
			bool		deletePrevMesh = false;

			// Execute the requested action on the chunk mesh
			switch (request.second) {
				case ChunkAction::CREATE_UPDATE:
					newMesh = _generateMesh(data, neightboursChunks, 1, deletePrevMesh);
					if (newMesh)
						localMeshes[data.Wpos].mesh = newMesh;
					if (deletePrevMesh)
						localDeletion.push_back(data.Wpos);
					break;

				case ChunkAction::DELETE:
					if (_deleteMesh(data, neightboursChunks))
						localDeletion.push_back(data.Wpos);
					break;
			}

			data.inCreation = false;
		}

		// Chunk data is deleted here to avoid missing needed chunk data while deleting meshes
		for (ChunkMap::iterator it = _chunks.begin(); it != _chunks.end(); it++) {
			if (!it->second.chunk) {
				ChunkMap::iterator	next = it; next++;
				_chunks.erase(it);
				it = next;
			}
			if (it == _chunks.end())
				break ;
		}

		_chunksMutex.unlock();
		
		_meshesMutex.lock();
		_meshesToDeleteMutex.lock();

		for (list<glm::ivec3>::iterator it = localDeletion.begin(); it != localDeletion.end(); it++) {
			_meshesToDelete.push_back(_meshes[*it].mesh);
			_meshes.erase(*it);
		}

		_meshesToDeleteMutex.unlock();

		
		for (MeshMap::iterator it = localMeshes.begin(); it != localMeshes.end(); it++) {
			if (!_meshes.count(it->first))
				_meshes[it->first].mesh = it->second.mesh;
		}
		
		_meshesMutex.unlock();

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
ChunkMesh *	VoxelSystem::_generateMesh(ChunkData &chunk, ChunkData *neightboursChunks[6], const uint8_t &LOD, bool &deletePrevMesh) {
	// Check if the chunk already have a mesh (in case of update)
	if (_meshes.count(chunk.Wpos) && _meshes[chunk.Wpos].mesh) {
		if (_deleteMesh(chunk, neightboursChunks))
			deletePrevMesh = true;
	}

	chunk.neigthbourUpdated = false;

	// Check if the chunk completely empty
	if (!chunk.chunk || (IS_CHUNK_COMPRESSED(chunk.chunk) && !BLOCK_AT(chunk.chunk, 0, 0, 0)))
		return nullptr;

	vector<DATA_TYPE>	vertices;
	size_t		maxDataSizePerChunk = (pow(CHUNK_SIZE, 3) / 2) * 6 * sizeof(DATA_TYPE);

	vertices.reserve(maxDataSizePerChunk);

	_constructChunkMesh(&vertices, chunk, neightboursChunks, LOD);

	return new ChunkMesh(vertices);
}

// Delete the first mesh
bool	VoxelSystem::_deleteMesh(ChunkData &chunk, ChunkData *neightboursChunks[6]) {
	if (_chunks.find(chunk.Wpos) == _chunks.end() || (_meshes.count(chunk.Wpos) && !_meshes[chunk.Wpos].mesh))
		return false;

	if (chunk.neigthbourUpdated)
		return true;

	// Request the update of neighbouring chunks
	list<ChunkRequest>	neightboursRequests;

	for (size_t i = 0; i < 6; i++) {
		if (neightboursChunks[i]) {
			neightboursChunks[i]->neigthbourUpdated = true;
			neightboursRequests.push_back({neightboursChunks[i]->Wpos, ChunkAction::CREATE_UPDATE});
		}
	}

	requestMesh(neightboursRequests);
	return true;
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
