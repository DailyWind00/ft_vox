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

		deque<ChunkRequest> localRequestedChunks = _requestedChunks;
		_requestedChunksMutex.unlock();


		// Generate/delete chunks up to the batch limit
		size_t			batchCount = 0;
		ChunkMap 		generatedChunks;
		deque<ivec3>	chunksToDelete;

		for (const ChunkRequest &req : localRequestedChunks) {
			if (batchCount++ >= BATCH_LIMIT)
				break;

			ivec3 pos = req.first;

			// Execute the requested action on the chunk in local memory
			switch (req.second) {
				case ChunkAction::CREATE_UPDATE:
					generatedChunks[pos] = {ChunkHandler::createChunk(pos), MAX_LOD, pos};
					break;

				case ChunkAction::DELETE:
					chunksToDelete.push_back(pos);
					break;
			}
		}


		vector<ChunkRequest> meshRequests;
		meshRequests.reserve(batchCount);
		_chunksMutex.lock();

		// Put the generated chunks in the shared memory and request their meshes
		for (const auto& chunk : generatedChunks) {
			_generateChunk(chunk.first);
			meshRequests.push_back({chunk.first, ChunkAction::CREATE_UPDATE});
		}
		
		// Delete the chunks from the shared memory and request the deletion of their meshes
		for (const ivec3 &pos : chunksToDelete) {
			_deleteChunk(pos);
			meshRequests.push_back({pos, ChunkAction::DELETE});
		}

		_chunksMutex.unlock();


		// Remove the generated chunks from the requested list
		_requestedChunksMutex.lock();
		_requestedChunks.erase(_requestedChunks.begin(), _requestedChunks.begin() + batchCount);
		_requestedChunksMutex.unlock();


		// Request the mesh generation
		requestMesh(meshRequests);
	}

	if (VERBOSE)
		cout << "Chunk Generation thread stopped" << endl;
}

// Generate a new chunk
void VoxelSystem::_generateChunk(const ivec3 &pos) {
	if (_chunks.count(pos))
		return;

	_chunks[pos] = {ChunkHandler::createChunk(pos), MAX_LOD, pos};
}

// Delete a chunk
void VoxelSystem::_deleteChunk(const ivec3 &pos) {
	if (!_chunks.count(pos))
		return;

	delete _chunks[pos].chunk;
	_chunks.erase(pos);
}
/// ---



/// Public functions

// Request an action on a chunk :
//  - CREATE_UPDATE will generate the chunk and request a mesh creation
//  - DELETE will delete the chunk and request the deletion of the mesh
void	VoxelSystem::requestChunk(const vector<ChunkRequest> &requests) {
	if (!requests.size())
		return;

	_requestedChunksMutex.lock();

	for (ChunkRequest req : requests) {
		// Check if the request already exists
		if (find(_requestedChunks.begin(), _requestedChunks.end(), req) == _requestedChunks.end())
			_requestedChunks.push_back(req);
	}

	_requestedChunksMutex.unlock();
}
/// ---