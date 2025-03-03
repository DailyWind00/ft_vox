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

		vector<MeshRequest> localRequestedMeshes = _requestedMeshes;
		_requestedMeshesMutex.unlock();


		// Generate meshes up to the batch limit
		size_t batchCount = 0;
		_chunksMutex.lock();

		for (MeshRequest request : localRequestedMeshes) {
			if (request.second == ChunkAction::NONE)
				continue;

			ivec3	Wpos = request.first;

			// Calculate the LOD of the chunk
			// const vec3 &	camPos = _camera.getCameraInfo().position;
			// const size_t	dist   = glm::distance(camPos, (vec3)Wpos);
			// const size_t	LOD    = glm::min(((dist >> 5) + MAX_LOD), MIN_LOD); // +1 LOD every 32 chunks

			_chunks[Wpos].LOD = 1;
			ChunkData data = _chunks[Wpos];


			// Search for neightbouring chunks
			const ivec3	neightboursPos[6] = { 
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
				case ChunkAction::CREATE_UPDATE: _generateMesh(data, neightboursChunks); break;
				case ChunkAction::DELETE: _deleteChunk(data, neightboursChunks); break;
				case ChunkAction::LOAD:   _loadMesh(data, neightboursChunks);    break;
				case ChunkAction::UNLOAD: _unloadMesh(data, neightboursChunks);  break;

				default:
					throw std::runtime_error("Invalid ChunkAction");
			}
			
			batchCount++;
			if (batchCount >= BATCH_LIMIT)
				break;
		}

		_chunksMutex.unlock();
		_buffersNeedUpdates = ChunkAction::CREATE_UPDATE;


		// Remove the generated meshes from the requested list
		_requestedMeshesMutex.lock();
		_requestedMeshes.erase(_requestedMeshes.begin(), _requestedMeshes.begin() + std::min(batchCount, _requestedMeshes.size()));
		_requestedMeshesMutex.unlock();
	}

	if (VERBOSE)
		cout << "Mesh Generation thread stopped" << endl;
}

// Check if the voxel at the given position is visible
// Return a bitmask of the visible faces (6 bits : ZzYyXx)
static uint8_t	isVoxelVisible(const ivec3 &Vpos, const ChunkData &chunk, ChunkData *neightboursChunks[6]) {
	AChunk *		data = chunk.chunk;
	const size_t &	LOD = chunk.LOD;
	const size_t &	x = Vpos.x;
	const size_t &	y = Vpos.y;
	const size_t &	z = Vpos.z;

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
void	VoxelSystem::_generateMesh(const ChunkData &chunk, ChunkData *neightboursChunks[6]) {
	// Check if the chunk completely empty
	if (IS_CHUNK_COMPRESSED(chunk.chunk) && !BLOCK_AT(chunk.chunk, 0, 0, 0))
		return;

	vector<DATA_TYPE>	vertices[6];

	// Generate vertices for visible faces
	for (size_t x = 0; x < CHUNK_SIZE; x += 1 * chunk.LOD) {
		for (size_t y = 0; y < CHUNK_SIZE; y += 1 * chunk.LOD) {
			if (IS_LAYER_COMPRESSED(chunk.chunk, y) && !BLOCK_AT(chunk.chunk, x, y, 0))
				continue; // Void layer

			for (size_t z = 0; z < CHUNK_SIZE; z += 1 * chunk.LOD) {

				uint8_t visibleFaces = isVoxelVisible({x, y, z}, chunk, neightboursChunks);
				if (!visibleFaces)
					continue;

				for (int i = 0; i < 6; i++) {
					if (visibleFaces & (1 << i)) {
						DATA_TYPE data = 0;

						// Encode data
						data |= (x & 0x1F);     	// 5 bits for x
						data |= (y & 0x1F) << 5;	// 5 bits for y
						data |= (z & 0x1F) << 10;	// 5 bits for z

						data |= (BLOCK_AT(chunk.chunk, x, y, z) & 0x7F) << 15;	// 7 bits for block ID

						data |= (1 & 0x1F) << 22;	// 5 bits for length
						data |= (1 & 0x1F) << 27;	// 5 bits for length

						vertices[i].push_back(data);
					}
				}
			}
		}
	}

	// TODO : greedy meshing here

	// Wait for the buffers to be available
	while (_buffersNeedUpdates != ChunkAction::NONE && !_quitting)
		this_thread::sleep_for(chrono::milliseconds(THREAD_SLEEP_DURATION));

	// Write the mesh in OpenGL buffers
	for (int i = 0; i < 6; i++) {
		if (!vertices[i].size())
			continue;

		_VBO_data.insert(_VBO_data.end(), vertices[i].begin(), vertices[i].end());
		_IB_data.push_back( DrawCommand{4, (GLuint)vertices[i].size(), 0, (GLuint)_VBO_data.size()} );
		_SSBO_data.push_back( SSBOData{{chunk.Wpos, i}} );
	}
}

// Delete a chunk and its mesh
void	VoxelSystem::_deleteChunk(const ChunkData &chunk, ChunkData *neightboursChunks[6]) {
	cout << "Deleting chunk at " << chunk.Wpos.x << " " << chunk.Wpos.y << " " << chunk.Wpos.z << endl;
	// TODO
	(void)neightboursChunks;
}

// Load a chunk (add the drawcall to the batched rendering)
void	VoxelSystem::_loadMesh(const ChunkData &chunk, ChunkData *neightboursChunks[6]) {
	cout << "Loading chunk at " << chunk.Wpos.x << " " << chunk.Wpos.y << " " << chunk.Wpos.z << endl;
	// TODO
	(void)neightboursChunks;
}

// Unload a chunk (remove the drawcall from the batched rendering)
void	VoxelSystem::_unloadMesh(const ChunkData &chunk, ChunkData *neightboursChunks[6]) {
	cout << "Unloading chunk at " << chunk.Wpos.x << " " << chunk.Wpos.y << " " << chunk.Wpos.z << endl;
	// TODO
	(void)neightboursChunks;
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

	chunkRequests.reserve(Wpositions.size());
	_requestedMeshes.reserve(Wpositions.size());

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