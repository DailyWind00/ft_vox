#include "VoxelSystem.hpp"

/// Private functions

// Chunk Generation thread routine
void VoxelSystem::_chunkGenerationRoutine() {
	if (VERBOSE)
		cout << "Chunk Generation thread started" << endl;

	while (!_quitting) {

		// Check if there are chunks to generate, sleep if not
		if (!_requestedChunksMutex.try_lock() || !_requestedChunks.size()) {
			_requestedChunksMutex.unlock();
			this_thread::sleep_for(chrono::milliseconds(THREAD_SLEEP_DURATION));
			continue;
		}

		// Generate chunks up to the batch limit
		int batchCount = 0;
		_chunksMutex.lock();

		for (ivec3 pos : _requestedChunks) {
			if (_chunks.count(pos))
				continue;

			_chunks[pos] = ChunkHandler::createChunk(pos);

			_requestedMeshes.push_back(pos);

			batchCount++;
			if (batchCount >= BATCH_LIMIT)
				break;
		}
		
		_chunksMutex.unlock();

		// Add the generated chunks to the requested meshes list
		_requestedMeshesMutex.lock();
		_requestedMeshes.insert(_requestedMeshes.end(), _requestedChunks.begin(), _requestedChunks.begin() + batchCount);
		_requestedMeshesMutex.unlock();

		// Remove the generated chunks from the requested list
		_requestedChunks.erase(_requestedChunks.begin(), _requestedChunks.begin() + batchCount);
		
		_requestedChunksMutex.unlock();
	}

	if (VERBOSE)
		cout << "Chunk Generation thread stopped" << endl;
}
/// ---



/// Public functions

// Request the generation of a chunk
// A finished chunk will also request a mesh update
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