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
		_chunksMutex.lock();
		_buffersMutex.lock();

		for (ChunkRequest request : localRequestedMeshes) {
			if (batchCount >= BATCH_LIMIT)
				break;

			batchCount++;

			ivec3	Wpos = request.first;
			if (_chunks.find(Wpos) == _chunks.end())
				continue;

			// Calculate the LOD of the chunk (cause crashes for now)
			// const vec3 &	camPos = _camera.getCameraInfo().position;
			// const size_t	dist   = glm::distance(camPos, (vec3)Wpos);
			// const size_t	LOD    = glm::min(((dist >> 5) + MAX_LOD), MIN_LOD); // +1 LOD every 32 chunks

			// _chunks[Wpos].LOD = LOD;
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


			// Execute the requested action on the chunk mesh
			switch (request.second) {
				case ChunkAction::CREATE_UPDATE:
					_generateMesh(data, neightboursChunks);
					break;

				case ChunkAction::DELETE:
					_deleteMesh(data, neightboursChunks);
					break;
			}

			data.inCreation = false;
		}

		_buffersNeedUpdates = true;
		_chunksMutex.unlock();
		_buffersMutex.unlock();


		// Remove the generated meshes from the requested list
		_requestedMeshesMutex.lock();
		_requestedMeshes.erase(_requestedMeshes.begin(), _requestedMeshes.begin() + batchCount);
		_requestedMeshesMutex.unlock();
	}

	if (VERBOSE)
		cout << "Mesh Generation thread stopped" << endl;
}

// Check if a neighbour chunk mesh is loaded
static inline bool	isNeightbourLoaded(ChunkData *neightbour) {
	return neightbour && neightbour->chunk && (neightbour->hasMesh() || neightbour->inCreation);
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

	// TODO : Compact & optimize this code
	for (const ivec3 &pos : neightbours) {
		/// X axis
		// Border
		if (pos.x < 0) {
			if (!isNeightbourLoaded(neightboursChunks[4]) || (neightboursChunks[4]->chunk && !BLOCK_AT(neightboursChunks[4]->chunk, CHUNK_SIZE - LOD, pos.y, pos.z)))
				visibleFaces |= (1 << 0);
		}
		else if (pos.x >= CHUNK_SIZE) {
			if (!isNeightbourLoaded(neightboursChunks[5]) || (neightboursChunks[5]->chunk && !BLOCK_AT(neightboursChunks[5]->chunk, 0, pos.y, pos.z)))
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
			if (!isNeightbourLoaded(neightboursChunks[2]) || (neightboursChunks[2]->chunk && !BLOCK_AT(neightboursChunks[2]->chunk, pos.x, CHUNK_SIZE - LOD, pos.z)))
				visibleFaces |= (1 << 2);
		}
		else if (pos.y >= CHUNK_SIZE) {
			if (!isNeightbourLoaded(neightboursChunks[3]) || (neightboursChunks[3]->chunk && !BLOCK_AT(neightboursChunks[3]->chunk, pos.x, 0, pos.z)))
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
			if (!isNeightbourLoaded(neightboursChunks[0]) || (neightboursChunks[0]->chunk && !BLOCK_AT(neightboursChunks[0]->chunk, pos.x, pos.y, CHUNK_SIZE - LOD)))
				visibleFaces |= (1 << 4);
		}
		else if (pos.z >= CHUNK_SIZE) {
			if (!isNeightbourLoaded(neightboursChunks[1]) || (neightboursChunks[1]->chunk && !BLOCK_AT(neightboursChunks[1]->chunk, pos.x, pos.y, 0)))
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

// Create/update the mesh of the given chunk
// The data will be stored in the main thread at the end of OpenGL buffers
void	VoxelSystem::_generateMesh(ChunkData &chunk, ChunkData *neightboursChunks[6]) {
	// Check if the chunk already have a mesh (in case of update)
	if (chunk.VBO_area[1] && chunk.IB_area[1] && chunk.SSBO_area[1])
		_deleteMesh(chunk, neightboursChunks);

	chunk.neigthbourUpdated = false;

	// Check if the chunk completely empty
	if (!chunk.chunk || (IS_CHUNK_COMPRESSED(chunk.chunk) && !BLOCK_AT(chunk.chunk, 0, 0, 0)))
		return;

	vector<DATA_TYPE>	vertices[6];

	// Generate vertices for visible faces
	for (size_t x = 0; x < CHUNK_SIZE; x += chunk.LOD) {
		for (size_t y = 0; y < CHUNK_SIZE; y += chunk.LOD) {
			if (IS_LAYER_COMPRESSED(chunk.chunk, y) && !BLOCK_AT(chunk.chunk, x, y, 0))
				continue; // Void layer

			for (size_t z = 0; z < CHUNK_SIZE; z += chunk.LOD) {

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

						data |= ((1 - 1) & 0x1F) << 22;	// 5 bits for length (1 by default)
						data |= ((1 - 1) & 0x1F) << 27;	// 5 bits for length (1 by default)

						vertices[i].push_back(data);
					}
				}
			}
		}
	}

	// TODO : greedy meshing here

	size_t old_VBO_data_size = _VBO_data.size() * sizeof(DATA_TYPE);
	size_t old_IB_data_size = _IB_data.size() * sizeof(DrawCommand);
	size_t old_SSBO_data_size = _SSBO_data.size() * sizeof(SSBOData);

	// Write the mesh in OpenGL buffers
	for (int i = 0; i < 6; i++) {
		if (!vertices[i].size())
			continue;

		// IB first as it need the VBO size before it being written
		DrawCommand	cmd = {4, GLuint(vertices[i].size()), 0, GLuint((_VBO_size / sizeof(DATA_TYPE)) + _VBO_data.size())};
		_IB_data.push_back(cmd);

		// VBO
		_VBO_data.insert(_VBO_data.end(), vertices[i].begin(), vertices[i].end());

		// SSBO
		SSBOData data = {ivec4{chunk.Wpos, i}};
		_SSBO_data.push_back(data);
	}

	// Update the chunk data
	if (!chunk.VBO_area[1] || chunk.VBO_area[1] > _VBO_data.size() * sizeof(DATA_TYPE) - old_VBO_data_size) {
		chunk.VBO_area[0] = old_VBO_data_size + _VBO_size;
		chunk.VBO_area[1] = _VBO_data.size() * sizeof(DATA_TYPE) - old_VBO_data_size;
	}

	if (!chunk.IB_area[1]) {
		chunk.IB_area[0] = old_IB_data_size + _IB_size;
		chunk.IB_area[1] = _IB_data.size() * sizeof(DrawCommand) - old_IB_data_size;
	}

	if (!chunk.SSBO_area[1]) {
		chunk.SSBO_area[0] = old_SSBO_data_size + _SSBO_size;
		chunk.SSBO_area[1] = _SSBO_data.size() * sizeof(SSBOData) - old_SSBO_data_size;
	}

	{ // to remove
		cout << "Generated mesh at " << chunk.Wpos.x << " " << chunk.Wpos.y << " " << chunk.Wpos.z << endl;
		cout << "VBO  : " << chunk.VBO_area[0] << " " << chunk.VBO_area[1] << endl;
		cout << "IB   : " << chunk.IB_area[0] << " " << chunk.IB_area[1] << endl;
		cout << "SSBO : " << chunk.SSBO_area[0] << " " << chunk.SSBO_area[1] << endl << endl;
	} // --
}

// Delete the first mesh
void	VoxelSystem::_deleteMesh(ChunkData &chunk, ChunkData *neightboursChunks[6]) {
	if (!_chunks.count(chunk.Wpos) || !chunk.hasMesh())
		return;

	cout << "Deleting mesh at " << chunk.Wpos.x << " " << chunk.Wpos.y << " " << chunk.Wpos.z << endl;

	_chunksToDelete.push_back(chunk.Wpos);

	if (chunk.neigthbourUpdated)
		return;

	// Request the update of neighbouring chunks
	vector<ChunkRequest>	neightboursRequests;

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
void	VoxelSystem::requestMesh(const vector<ChunkRequest> &requests) {
	if (!requests.size())
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