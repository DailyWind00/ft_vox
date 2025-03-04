#include "VoxelSystem.hpp"

/// Private functions

// Chunk Generation thread routine
void VoxelSystem::_chunkGenerationRoutine() {
	if (VERBOSE)
		cout << "> Chunk Generation thread started" << endl;

	while (!_quitting) {

		// Check if there are chunks to generate, sleep if not
		if (!_requestedChunks.size() || !_requestedChunksMutex.try_lock()) {
			this_thread::sleep_for(chrono::milliseconds(THREAD_SLEEP_DURATION));
			continue;
		}

		deque<ivec3> localRequestedChunks = _requestedChunks;
		_requestedChunksMutex.unlock();


		// Generate chunks up to the batch limit
		size_t batchCount = 0;
		ChunkMap generatedChunks;
		
		for (const ivec3 &pos : localRequestedChunks) {
			if (batchCount >= BATCH_LIMIT)
				break;

			batchCount++;

			if (_chunks.count(pos)) // Check if the chunk already exists
				continue;

			generatedChunks[pos] = {ChunkHandler::createChunk(pos), 1, pos};
		}


		// Add the generated chunks to the ChunkMap and request their meshes
		vector<MeshRequest> meshRequests;
		meshRequests.reserve(batchCount);

		_chunksMutex.lock();

		for (const auto &chunk : generatedChunks) {
			_chunks[chunk.first] = chunk.second;
			meshRequests.push_back({chunk.first, ChunkAction::CREATE_UPDATE});
		}

		_chunksMutex.unlock();


		// Remove the generated chunks from the requested list
		_requestedChunksMutex.lock();
		while (batchCount--)
			_requestedChunks.pop_front();
		_requestedChunksMutex.unlock();


		// Request the mesh generation
		_requestedMeshesMutex.lock();
		_requestedMeshes.insert(_requestedMeshes.end(), meshRequests.begin(), meshRequests.end());
		_requestedMeshesMutex.unlock();	
	}

	if (VERBOSE)
		cout << "Chunk Generation thread stopped" << endl;
}
/// ---



/// Public functions

// Request the generation of a chunk
// A finished chunk will also request a mesh creation
void	VoxelSystem::requestChunk(const vector<ivec3> &Wpositions) {
	if (!Wpositions.size())
		return;

	_requestedChunksMutex.lock();

	for (ivec3 pos : Wpositions) {
		if (_chunks.count(pos) || find(_requestedChunks.begin(), _requestedChunks.end(), pos) == _requestedChunks.end())
			_requestedChunks.push_back(pos);
	}

	_requestedChunksMutex.unlock();
}
/// ---