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

		// duplicate requested chunks up to the batch limit
		deque<ChunkRequest>	localRequestedChunks;
		size_t			batchCount = 0;
		
		for (; batchCount < CHUNK_BATCH_LIMIT / (_cpuCoreCount / CHUNKGEN_CORE_RATIO) && _requestedChunks.size(); batchCount++) {
			auto	tmp = _requestedChunks.begin();

			if (tmp == _requestedChunks.end())
				break ;

			ChunkRequest	newReq = *tmp;
			_requestedChunks.pop_front();
			localRequestedChunks.push_back(newReq);
		}

		_requestedChunksMutex.unlock();


		// Generate/delete chunks that have been duplicated locally
		ChunkMap 		generatedChunks;
		deque<ivec3>	chunksToDelete;

		for (const ChunkRequest &req : localRequestedChunks) {
			ivec3 pos = req.first;

			// Execute the requested action on the chunk in local memory
			switch (req.second) {
				case ChunkAction::CREATE_UPDATE:
					generatedChunks[pos] = ChunkData{ChunkHandler::createChunk(pos), pos, MAX_LOD};
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
		for (ChunkMap::value_type &chunk : generatedChunks) {
			_generateChunk(chunk);
			meshRequests.push_back({chunk.first, ChunkAction::CREATE_UPDATE});
		}
		
		// Delete the chunks from the shared memory and request the deletion of their meshes
		for (const ivec3 &pos : chunksToDelete) {
			_deleteChunk(pos);
			meshRequests.push_back({pos, ChunkAction::DELETE});
		}

		_chunksMutex.unlock();

		// Request the mesh generation
		requestMesh(meshRequests);
	}

	if (VERBOSE)
		cout << "Chunk Generation thread stopped" << endl;
}

// Generate a new chunk
void VoxelSystem::_generateChunk(ChunkMap::value_type &chunk) {
	if (_chunks.count(chunk.first))
		return;

	_chunks[chunk.first] = chunk.second;
}

// Delete a chunk
// It will be removed from the ChunkMap in the main thread
void VoxelSystem::_deleteChunk(const ivec3 &pos) {
	if (!_chunks.count(pos))
		return;

	delete _chunks[pos].chunk;
	_chunks[pos].chunk = nullptr;
}

void	VoxelSystem::_chunkFloodFill(const glm::ivec3 &pos, const glm::ivec3 &oldPos, const ChunkAction &reqType, vector<ChunkRequest> *requests)
{
	if (abs(pos.x - oldPos.x) + abs(pos.z - oldPos.z) > HORIZONTAL_RENDER_DISTANCE || abs(pos.y - oldPos.y) >= VERTICAL_RENDER_DISTANCE)
		return ;
	if (std::find(requests->begin(), requests->end(), std::pair(pos, reqType)) == requests->end()) {
		requests->push_back(std::pair(pos, reqType));
		_chunkFloodFill({pos.x + 1, pos.y, pos.z}, oldPos, reqType, requests);
		_chunkFloodFill({pos.x - 1, pos.y, pos.z}, oldPos, reqType, requests);
		_chunkFloodFill({pos.x, pos.y, pos.z + 1}, oldPos, reqType, requests);
		_chunkFloodFill({pos.x, pos.y, pos.z - 1}, oldPos, reqType, requests);
		// _chunkFloodFill({pos.x, pos.y + 1, pos.z}, oldPos, reqType, requests);
		// _chunkFloodFill({pos.x, pos.y - 1, pos.z}, oldPos, reqType, requests);
	}
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
