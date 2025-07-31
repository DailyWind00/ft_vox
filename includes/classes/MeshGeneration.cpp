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
			// _chunks[Wpos].LOD   = glm::min(((dist >> 5) + MAX_LOD), MIN_LOD); // +1 LOD every 32 chunks

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

// Update the area of the given buffer
// Use of template to avoid issues with the different buffer types
template<typename AreaType>
static inline void updateArea(AreaType &area, size_t old_size, size_t current_size, size_t buffer_offset) {
	// if (!area.size || area.size > current_size - old_size) {
		area.offset = old_size + buffer_offset;
		area.size   = current_size - old_size;
	// }
}

// Create/update the mesh of the given chunk
// The data will be stored in the main thread at the end of OpenGL buffers
void	VoxelSystem::_generateMesh(ChunkData &chunk, ChunkData *neightboursChunks[6]) {
	// Check if the chunk already have a mesh (in case of update)
	if (chunk.hasMesh())
		_deleteMesh(chunk, neightboursChunks);

	chunk.neigthbourUpdated = false;

	// Check if the chunk completely empty
	if (!chunk.chunk || (IS_CHUNK_COMPRESSED(chunk.chunk) && !BLOCK_AT(chunk.chunk, 0, 0, 0)))
		return;

	uint64_t	xAxisBitmask[(CHUNK_WIDTH + 2) * (CHUNK_HEIGHT + 2)] = {0};
	uint64_t	yAxisBitmask[(CHUNK_WIDTH + 2) * (CHUNK_WIDTH + 2)] = {0};
	uint64_t	zAxisBitmask[(CHUNK_WIDTH + 2) * (CHUNK_HEIGHT + 2)] = {0};

	// Set the bitmasks of all the axis
	for (uint64_t y = 0; y < CHUNK_HEIGHT; y++) {
		for (uint64_t z = 0; z < CHUNK_WIDTH; z++) {
			for (uint64_t x = 0; x < CHUNK_WIDTH; x++) {
				// For each axis, write a bit to represent a solid block
				if (BLOCK_AT(chunk.chunk, x, y, z) != 0) {
					xAxisBitmask[y * CHUNK_HEIGHT + z] |= (uint64_t)0x1 << (x + 1);	// Bit-shift is offset by one to allow for neighbour data
					yAxisBitmask[z * CHUNK_WIDTH + x] |= (uint64_t)0x1 << (y + 1);	// Bit-shift is offset by one to allow for neighbour data
					zAxisBitmask[y * CHUNK_HEIGHT + x] |= (uint64_t)0x1 << (z + 1);	// Bit-shift is offset by one to allow for neighbour data
				}
			}
		}
	}

	// Get the X axis neighbours data
	for (uint64_t i = 0; i < CHUNK_HEIGHT * CHUNK_WIDTH; i++) {
		uint64_t	y = i / CHUNK_WIDTH;
		uint64_t	z = i % CHUNK_WIDTH;
		if (isNeightbourLoaded(neightboursChunks[4]) && BLOCK_AT(neightboursChunks[4]->chunk, CHUNK_WIDTH - 1, y, z))
			xAxisBitmask[i] |= (uint64_t)0x1 << 0;
		if (isNeightbourLoaded(neightboursChunks[5]) && BLOCK_AT(neightboursChunks[5]->chunk, 0, y, z))
			xAxisBitmask[i] |= (uint64_t)0x1 << (CHUNK_WIDTH + 1);
	}

	// Get the Y axis neighbours data
	for (uint64_t i = 0; i < CHUNK_WIDTH * CHUNK_WIDTH; i++) {
		uint64_t	x = i % CHUNK_WIDTH;
		uint64_t	z = i / CHUNK_WIDTH;
		if (isNeightbourLoaded(neightboursChunks[2]) && BLOCK_AT(neightboursChunks[2]->chunk, x, CHUNK_HEIGHT - 1, z))
			yAxisBitmask[i] |= (uint64_t)0x1 << 0;
		if (isNeightbourLoaded(neightboursChunks[3]) && BLOCK_AT(neightboursChunks[3]->chunk, x, 0, z))
			yAxisBitmask[i] |= (uint64_t)0x1 << (CHUNK_HEIGHT + 1);
	}

	// Get the Z axis neighbours data
	for (uint64_t i = 0; i < CHUNK_HEIGHT * CHUNK_WIDTH; i++) {
		uint64_t	x = i % CHUNK_WIDTH;
		uint64_t	y = i / CHUNK_HEIGHT;
		if (isNeightbourLoaded(neightboursChunks[0]) && BLOCK_AT(neightboursChunks[0]->chunk, x, y, CHUNK_WIDTH - 1))
			zAxisBitmask[i] |= (uint64_t)0x1 << 0;
		if (isNeightbourLoaded(neightboursChunks[1]) && BLOCK_AT(neightboursChunks[1]->chunk, x, y, 0))
			zAxisBitmask[i] |= (uint64_t)0x1 << (CHUNK_WIDTH + 1);
	}

	// Temporary hack to get basic meshing working with the new culling system (WILL GET REPLACED SOON !!)
	vector<DATA_TYPE>	vertices[6];
	for (uint64_t i = 0; i < CHUNK_WIDTH * CHUNK_WIDTH; i++) {
		uint64_t	col = xAxisBitmask[i];
		// Culling facing forwardX
		uint64_t	forwardXCol = col & ~(col >> 1);
		forwardXCol = forwardXCol >> 1;
		forwardXCol = forwardXCol & ~((uint64_t)1 << CHUNK_WIDTH);
		// Culling facing backwardX
		uint64_t	backwardXCol = col & ~(col << 1);
		backwardXCol = backwardXCol >> 1;
		backwardXCol = backwardXCol & ~((uint64_t)1 << CHUNK_WIDTH);
		for (int j = 0; j < CHUNK_WIDTH; j++) {
			uint8_t	id = BLOCK_AT(chunk.chunk, j, i / CHUNK_WIDTH, i % CHUNK_WIDTH);
			if (1 & (forwardXCol >> j)) {
				DATA_TYPE	data = 0;
				data |= ((j) & 0x1F);			// X
				data |= ((i / CHUNK_WIDTH) & 0x1F) << 5;	// Y
				data |= ((i % CHUNK_WIDTH) & 0x1F) << 10;	// Z
				data  |= (id & 0x7F) << 15;			// Block ID
				data |= (0 & 0x1F) << 22;			// Face length
				data |= (0 & 0x1F) << 27;			// Face length
				vertices[1].push_back(data);
			}
			if (1 & (backwardXCol >> j)) {
				DATA_TYPE	data = 0;
				data |= ((j) & 0x1F);			// X
				data |= ((i / CHUNK_WIDTH) & 0x1F) << 5;	// Y
				data |= ((i % CHUNK_WIDTH) & 0x1F) << 10;	// Z
				data  |= (id & 0x7F) << 15;			// Block ID
				data |= (0 & 0x1F) << 22;			// Face length
				data |= (0 & 0x1F) << 27;			// Face length
				vertices[0].push_back(data);
			}
		}
	}
	for (uint64_t i = 0; i < CHUNK_WIDTH * CHUNK_WIDTH; i++) {
		uint64_t	col = yAxisBitmask[i];
		// Culling facing forwardY
		uint64_t	forwardYCol = col & ~(col >> 1);
		forwardYCol = forwardYCol >> 1;
		forwardYCol = forwardYCol & ~((uint64_t)1 << CHUNK_WIDTH);
		// Culling facing backwardY
		uint64_t	backwardYCol = col & ~(col << 1);
		backwardYCol = backwardYCol >> 1;
		backwardYCol = backwardYCol & ~((uint64_t)1 << CHUNK_WIDTH);
		for (int j = 0; j < CHUNK_WIDTH; j++) {
			uint8_t	id = BLOCK_AT(chunk.chunk, i % CHUNK_WIDTH, j, i / CHUNK_WIDTH);
			if (1 & (forwardYCol >> j)) {
				DATA_TYPE	data = 0;
				data |= ((i % CHUNK_WIDTH) & 0x1F);	// X
				data |= (j & 0x1F) << 5;		// Y
				data |= ((i / CHUNK_WIDTH) & 0x1F) << 10;// Z
				data  |= (id & 0x7F) << 15;		// Block ID
				data |= (0 & 0x1F) << 22;		// Face length
				data |= (0 & 0x1F) << 27;		// Face length
				vertices[3].push_back(data);
			}
			if (1 & (backwardYCol >> j)) {
				DATA_TYPE	data = 0;
				data |= ((i % CHUNK_WIDTH) & 0x1F);	// X
				data |= (j & 0x1F) << 5;		// Y
				data |= ((i / CHUNK_WIDTH) & 0x1F) << 10;// Z
				data |= (id & 0x7F) << 15;		// Block ID
				data |= (0 & 0x1F) << 22;		// Face length
				data |= (0 & 0x1F) << 27;		// Face length
				vertices[2].push_back(data);
			}
		}
	}
	for (uint64_t i = 0; i < CHUNK_WIDTH * CHUNK_WIDTH; i++) {
		uint64_t	col = zAxisBitmask[i];
		// Culling facing forwardY
		uint64_t	forwardZCol = col & ~(col >> 1);
		forwardZCol = forwardZCol >> 1;
		forwardZCol = forwardZCol & ~((uint64_t)1 << CHUNK_WIDTH);
		// Culling facing backwardY
		uint64_t	backwardZCol = col & ~(col << 1);
		backwardZCol = backwardZCol >> 1;
		backwardZCol = backwardZCol & ~((uint64_t)1 << CHUNK_WIDTH);
		for (int j = 0; j < CHUNK_WIDTH; j++) {
			uint8_t	id = BLOCK_AT(chunk.chunk, i % CHUNK_WIDTH, i / CHUNK_WIDTH, j);
			if (1 & (forwardZCol >> j)) {
				DATA_TYPE	data = 0;
				data |= ((i % CHUNK_WIDTH) & 0x1F);	// X
				data |= ((i / CHUNK_WIDTH) & 0x1F) << 5;// Y
				data |= ((j) & 0x1F) << 10;		// Z
				data |= (id & 0x7F) << 15;		// Block ID
				data |= (0 & 0x1F) << 22;		// Face length
				data |= (0 & 0x1F) << 27;		// Face length
				vertices[5].push_back(data);
			}
			if (1 & (backwardZCol >> j)) {
				DATA_TYPE	data = 0;
				data |= ((i % CHUNK_WIDTH) & 0x1F);	// X
				data |= ((i / CHUNK_WIDTH) & 0x1F) << 5;// Y
				data |= ((j) & 0x1F) << 10;		// Z
				data |= (id & 0x7F) << 15;		// Block ID
				data |= (0 & 0x1F) << 22;		// Face length
				data |= (0 & 0x1F) << 27;		// Face length
				vertices[4].push_back(data);
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

	// Create new areas for the buffers
	chunk.VBO_area.push_back({0, 0});
	chunk.IB_area.push_back({0, 0});
	chunk.SSBO_area.push_back({0, 0});

	// Update the areas of the buffers
	updateArea(chunk.VBO_area.back(), old_VBO_data_size, _VBO_data.size() * sizeof(DATA_TYPE), _VBO_size);
	updateArea(chunk.IB_area.back(), old_IB_data_size, _IB_data.size() * sizeof(DrawCommand), _IB_size);
	updateArea(chunk.SSBO_area.back(), old_SSBO_data_size, _SSBO_data.size() * sizeof(SSBOData), _SSBO_size);
}

// Delete the first mesh
void	VoxelSystem::_deleteMesh(ChunkData &chunk, ChunkData *neightboursChunks[6]) {
	if (!_chunks.count(chunk.Wpos) || !chunk.hasMesh())
		return;

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
