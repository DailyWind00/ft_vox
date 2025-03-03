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

		vector<ivec3> localRequestedChunks = _requestedChunks;
		_requestedChunksMutex.unlock();

		// Generate chunks up to the batch limit
		size_t batchCount = 0;
		ChunkMap generatedChunks;
		
		for (ivec3 pos : localRequestedChunks) {
			if (_chunks.count(pos))
				continue;

			generatedChunks[pos] = {ChunkHandler::createChunk(pos), 0, pos};

			batchCount++;
			if (batchCount >= BATCH_LIMIT)
				break;
		}
		
		// Add the generated chunks to the ChunkMap and request their meshes
		_chunksMutex.lock();
		_requestedMeshesMutex.lock();

		for (auto &chunk : generatedChunks) {
			_chunks[chunk.first] = chunk.second;
			_requestedMeshes.push_back({chunk.first, ChunkAction::CREATE_UPDATE});
		}

		_requestedMeshesMutex.unlock();
		_chunksMutex.unlock();

		// Remove the generated chunks from the requested list
		_requestedChunksMutex.lock();
		_requestedChunks.erase(_requestedChunks.begin(), _requestedChunks.begin() + std::min(batchCount, _requestedChunks.size()));
		_requestedChunksMutex.unlock();
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

	_requestedChunks.reserve(Wpositions.size());

	for (ivec3 pos : Wpositions) {
		if (_chunks.count(pos) || find(_requestedChunks.begin(), _requestedChunks.end(), pos) == _requestedChunks.end())
			_requestedChunks.push_back(pos);
	}

	_requestedChunksMutex.unlock();
}
/// ---