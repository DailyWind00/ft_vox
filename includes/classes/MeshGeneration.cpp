#include "VoxelSystem.hpp"

/// Static functions

// Check if the voxel at the given position is visible
// Return a bitmask of the visible faces (6 bits : ZzYyXx)
static uint8_t	isVoxelVisible(const ChunkData &chunk, ChunkData *neightboursChunks[6]) {
	AChunk *		data = chunk.chunk;
	const size_t &	LOD = chunk.LOD;
	const size_t &	x = chunk.Wpos.x;
	const size_t &	y = chunk.Wpos.y;
	const size_t &	z = chunk.Wpos.z;

	// Check if the voxel is solid
	if (!BLOCK_AT(data, x, y, z))
		return 0;

	// Define neightbouring blocks positions
	ivec3	neightbours[6] = {
		{x - 1 * LOD, y, z},
		{x + 1 * LOD, y, z},
		{x, y - 1 * LOD, z},
		{x, y + 1 * LOD, z},
		{x, y, z - 1 * LOD},
		{x, y, z + 1 * LOD}
	};

	// Check if the face is visible
	// Also check if the neightbours are in the same chunk or in another one
	uint8_t	visibleFaces = 0;

	for (const ivec3 &pos : neightbours) {
		/// X axis
		// Border
		if (pos.x < 0) {
			if (!neightboursChunks[4] || !BLOCK_AT(neightboursChunks[4]->chunk, CHUNK_SIZE - LOD, pos.y, pos.z))
				visibleFaces |= (1 << 0);
		}
		else if (pos.x >= CHUNK_SIZE) {
			if (!neightboursChunks[5] || !BLOCK_AT(neightboursChunks[5]->chunk, 0, pos.y, pos.z))
				visibleFaces |= (1 << 1);
		}
		// Inside
		else if (size_t(pos.x) < x && !BLOCK_AT(data, pos.x, pos.y, pos.z))
				visibleFaces |= (1 << 0);
		else if (size_t(pos.x) > x && !BLOCK_AT(data, pos.x, pos.y, pos.z))
				visibleFaces |= (1 << 1);


		/// y axis
		// Border
		if (pos.y < 0) {
			if (!neightboursChunks[2] || !BLOCK_AT(neightboursChunks[2]->chunk, pos.x, CHUNK_SIZE - LOD, pos.z))
				visibleFaces |= (1 << 2);
		}
		else if (pos.y >= CHUNK_SIZE) {
			if (!neightboursChunks[3] || !BLOCK_AT(neightboursChunks[3]->chunk, pos.x, 0, pos.z))
				visibleFaces |= (1 << 3);
		}
		// Inside
		else if (size_t(pos.y) < y && !BLOCK_AT(data, pos.x, pos.y, pos.z))
				visibleFaces |= (1 << 2);
		else if (size_t(pos.y) > y && !BLOCK_AT(data, pos.x, pos.y, pos.z))
				visibleFaces |= (1 << 3);


		/// z axis
		// Border
		if (pos.z < 0) {
			if (!neightboursChunks[0] || !BLOCK_AT(neightboursChunks[0]->chunk, pos.x, pos.y, CHUNK_SIZE - LOD))
				visibleFaces |= (1 << 4);
		}
		else if (pos.z >= CHUNK_SIZE) {
			if (!neightboursChunks[1] || !BLOCK_AT(neightboursChunks[1]->chunk, pos.x, pos.y, 0))
				visibleFaces |= (1 << 5);
		}
		// Inside
		else if (size_t(pos.z) < z && !BLOCK_AT(data, pos.x, pos.y, pos.z))
				visibleFaces |= (1 << 4);
		else if (size_t(pos.z) > z && !BLOCK_AT(data, pos.x, pos.y, pos.z))
				visibleFaces |= (1 << 5);
	}

	return visibleFaces;
}

// Create/update the mesh of the given chunk and store it in OpenGL buffers
static void	generateMesh(const ChunkData &chunk, ChunkData *neightboursChunks[6]) {
	cout << "Generating mesh for chunk at " << chunk.Wpos.x << " " << chunk.Wpos.y << " " << chunk.Wpos.z << endl;
	// TODO
}

// Delete a chunk and its mesh
static void	deleteChunk(const ChunkData &chunk, ChunkData *neightboursChunks[6]) {
	cout << "Deleting chunk at " << chunk.Wpos.x << " " << chunk.Wpos.y << " " << chunk.Wpos.z << endl;
	// TODO
}

// Load a chunk (add the drawcall to the batched rendering)
static void	loadChunk(const ChunkData &chunk, ChunkData *neightboursChunks[6]) {
	cout << "Loading chunk at " << chunk.Wpos.x << " " << chunk.Wpos.y << " " << chunk.Wpos.z << endl;
	// TODO
}

// Unload a chunk (remove the drawcall from the batched rendering)
static void	unloadChunk(const ChunkData &chunk, ChunkData *neightboursChunks[6]) {
	cout << "Unloading chunk at " << chunk.Wpos.x << " " << chunk.Wpos.y << " " << chunk.Wpos.z << endl;
	// TODO
}
/// ---



/// Private functions

// Mesh Generation thread routine
void	VoxelSystem::_meshGenerationRoutine() {
	if (VERBOSE)
		cout << "Mesh Generation thread started" << endl;

	while (!_quitting) {

		// Check if there are meshes to generate, sleep if not
		if (!_requestedMeshesMutex.try_lock() || !_requestedMeshes.size()) {
			_requestedMeshesMutex.unlock();
			this_thread::sleep_for(chrono::milliseconds(THREAD_SLEEP_DURATION));
			continue;
		}

		// Generate meshes up to the batch limit
		_chunksMutex.lock();

		int batchCount = 0;
		for (MeshRequest request : _requestedMeshes) {
			ivec3	Wpos = request.first;

			// Calculate the LOD of the chunk
			const vec3 &camPos = _camera.getCameraInfo().position;
			size_t		dist   = glm::distance(camPos, (vec3)Wpos);
			size_t		LOD    = glm::clamp(((dist >> 5) + MAX_LOD), MAX_LOD, MIN_LOD); // +1 LOD every 32 chunks

			_chunks[Wpos].LOD = LOD;
			ChunkData data = _chunks[Wpos];

			// Search for neightbouring chunks
			ivec3	neightboursPos[6] = { 
				{Wpos.x - 1, Wpos.y, Wpos.z}, {Wpos.x + 1, Wpos.y, Wpos.z}, // x axis
				{Wpos.x, Wpos.y - 1, Wpos.z}, {Wpos.x, Wpos.y + 1, Wpos.z}, // y axis
				{Wpos.x, Wpos.y, Wpos.z - 1}, {Wpos.x, Wpos.y, Wpos.z + 1}  // z axis
			};

			ChunkData	*neightboursChunks[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

			for (size_t i = 0; i < 6; i++) {
				if (_chunks.find(neightboursPos[i]) != _chunks.end())
					neightboursChunks[i] = &_chunks[neightboursPos[i]];
			}

			// Execute the requested action on the chunk mesh
			switch (request.second) {
				case ChunkAction::CREATE_UPDATE: generateMesh(data, neightboursChunks); break;
				case ChunkAction::DELETE: deleteChunk(data, neightboursChunks); break;
				case ChunkAction::LOAD:   loadChunk(data, neightboursChunks);   break;
				case ChunkAction::UNLOAD: unloadChunk(data, neightboursChunks); break;
			}
			
			batchCount++;
			if (batchCount >= BATCH_LIMIT)
				break;
		}
		_chunksMutex.unlock();

		// Remove the generated meshes from the requested list
		_requestedMeshes.erase(_requestedMeshes.begin(), _requestedMeshes.begin() + batchCount);

		_requestedMeshesMutex.unlock();
	}

	if (VERBOSE)
		cout << "Mesh Generation thread stopped" << endl;
}
/// ---



/// Public functions

// Request the update of a chunk mesh
// If a mesh is requested without a chunk, the chunk will be requested instead
void	VoxelSystem::requestMeshUpdate(const vector<ivec3> &Wpositions, const ChunkAction &action) {
	if (!Wpositions.size())
		return;

	_requestedMeshesMutex.lock();

	vector<ivec3>	chunkRequests;

	for (ivec3 pos : Wpositions) {
		if (!_chunks.count(pos))
			chunkRequests.push_back(pos);

		else if (find(_requestedMeshes.begin(), _requestedMeshes.end(), MeshRequest{pos, action}) == _requestedMeshes.end())
			_requestedMeshes.push_back({pos, action});
	}

	_requestedMeshesMutex.unlock();

	if (chunkRequests.size())
		requestChunk(chunkRequests);
}
/// ---