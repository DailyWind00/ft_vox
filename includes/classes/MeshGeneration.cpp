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
			if (batchCount >= MESH_BATCH_LIMIT)
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

// function to quickly count the number of trailing zeros in a 64-bit binary number representation
static int	trailing_zeros64(uint64_t x)
{
	int	n = 1;
	
	if (!x) return 64;
	if ((x & 0x00000000FFFFFFFF) == 0) {
		n = n + 32;
		x = x >> 32;
	}
	if ((x & 0x000000000000FFFF) == 0) {
		n = n + 16;
		x = x >> 16;
	}
	if ((x & 0x00000000000000FF) == 0) {
		n = n + 8;
		x = x >> 8;
	}
	if ((x & 0x000000000000000F) == 0) {
		n = n + 4;
		x = x >> 4;
	}
	if ((x & 0x0000000000000003) == 0) {
		n = n + 2;
		x = x >> 2;
	}
	return n - (x & 1);
}

// function to quickly count the number of trailing ones in a 64-bit binary number representation
static int	trailing_ones64(uint64_t x)
{
	return trailing_zeros64(~x);
}

// function to quickly count the number of trailing zeros in a 32-bit binary number representation
static int	trailing_zeros32(uint64_t x)
{
	int	n = 1;
	
	if (!x) return 32;
	if ((x & 0x0000FFFF) == 0) {
		n = n + 16;
		x = x >> 16;
	}
	if ((x & 0x000000FF) == 0) {
		n = n + 8;
		x = x >> 8;
	}
	if ((x & 0x0000000F) == 0) {
		n = n + 4;
		x = x >> 4;
	}
	if ((x & 0x00000003) == 0) {
		n = n + 2;
		x = x >> 2;
	}
	return n - (x & 1);
}

// function to quickly count the number of trailing ones in a 32-bit binary number representation
static int	trailing_ones32(uint64_t x)
{
	return trailing_zeros32(~x);
}
static DATA_TYPE	constructFace(const glm::ivec3 &pos, const uint8_t &blockID, const glm::ivec2 &size)
{
	DATA_TYPE	data = 0;

	data |= (pos.x & 0x1F);		// X
	data |= (pos.y & 0x1F) << 5;	// Y
	data |= (pos.z & 0x1F) << 10;	// Z
	data |= (blockID & 0x7F) << 15;	// Block ID
	data |= (size.x & 0x1F) << 22;	// Face length
	data |= (size.y & 0x1F) << 27;	// Face length
	return (data);
}

static uint64_t		binMap(const uint64_t &x, const uint64_t &count)
{
	int	res = 0;

	for (uint64_t i = 0; i < count; i++)
		res |= x << i;
	return (res);
}

// Binary greedy meshing algorythme
// Will quickly construte a mesh plane from a binary plane
static void	binaryGreedyMeshing(std::vector<DATA_TYPE> *vertices, uint32_t plane[CHUNK_WIDTH], const uint32_t &depth, const uint8_t &blockID, const uint8_t &axis)
{
	for (int i = 0; i < CHUNK_WIDTH; i++) {
		int	col = 0;

		while (col < CHUNK_WIDTH ) {
			col += trailing_zeros32(plane[i] >> col);
			if (col >= CHUNK_WIDTH)
				continue ;

			int	height = trailing_ones32(plane[i] >> col);
			int	width = 1;

			uint32_t	h = binMap(0x1, height);
			uint32_t	heightMask = h << col;

			while (i + width < CHUNK_WIDTH ) {
				uint64_t	next = (plane[i + width] >> col) & h;
				if (h != next)
					break ;
				plane[i + width] = plane[i + width] & ~heightMask;
				width++;
			}

			glm::ivec3	pos = {0, 0, 0};
			glm::ivec2	size = {0, 0};

			if (axis == 0) {
				pos = {depth, i, col};
				size = {height - 1, width - 1};
			}
			else if (axis == 1){
				pos = {col, depth, i};
				size = {width - 1, height - 1};
			}
			else if (axis == 2) {
				pos = {col, i, depth};
				size = {height - 1, width - 1};
			}

			vertices->push_back(constructFace(pos, blockID, size));
			col += height;
		}
	}
}

static void	constructXAxisMesh(std::vector<DATA_TYPE> (&vertices)[6], uint64_t (&xAxisBitmask)[(CHUNK_WIDTH + 2) * (CHUNK_WIDTH + 2)], ChunkData &chunk, ChunkData *neightboursChunks[6])
{
	// Get the X axis neighbours data
	for (uint64_t i = 0; i < CHUNK_HEIGHT * CHUNK_WIDTH; i++) {
		uint64_t	y = i / CHUNK_WIDTH;
		uint64_t	z = i % CHUNK_WIDTH;
		if (isNeightbourLoaded(neightboursChunks[4]) && BLOCK_AT(neightboursChunks[4]->chunk, CHUNK_WIDTH - 1, y, z))
			xAxisBitmask[i] |= (uint64_t)0x1 << 0;
		if (isNeightbourLoaded(neightboursChunks[5]) && BLOCK_AT(neightboursChunks[5]->chunk, 0, y, z))
			xAxisBitmask[i] |= (uint64_t)0x1 << (CHUNK_WIDTH + 1);
	}
	
	std::map<uint8_t, uint32_t[CHUNK_WIDTH][CHUNK_WIDTH]>	binaryPlaneHM[2];
	for (int i = 0; i < CHUNK_WIDTH * CHUNK_WIDTH; i++) {
		// Culling facing forwardX
		uint64_t	forwardXCol = xAxisBitmask[i] & ~(xAxisBitmask[i] >> 1);
		forwardXCol = forwardXCol >> 1;
		forwardXCol = forwardXCol & ~((uint64_t)1 << CHUNK_WIDTH);

		// Culling facing backwardX
		uint64_t	backwardXCol = xAxisBitmask[i] & ~(xAxisBitmask[i] << 1);
		backwardXCol = backwardXCol >> 1;
		backwardXCol = backwardXCol & ~((uint64_t)1 << CHUNK_WIDTH);
		
		for (int j = 0; j < CHUNK_WIDTH; j++) {
			uint8_t	id = BLOCK_AT(chunk.chunk, j, i / CHUNK_WIDTH, i % CHUNK_WIDTH);
			
			if (1 & (backwardXCol >> j))
				binaryPlaneHM[0][id][j][i / CHUNK_WIDTH] |= (uint32_t)0x1 << (i % CHUNK_WIDTH);

			if (1 & (forwardXCol >> j))
				binaryPlaneHM[1][id][j][i / CHUNK_WIDTH] |= (uint32_t)0x1 << (i % CHUNK_WIDTH);
		}
	}
	// Execute the binary greedy meshing algorythme for the X axis
	for (int i = 0; i < 2; i++)
		for (auto &[key, value] : binaryPlaneHM[i])
			for (int j = 0; j < CHUNK_WIDTH; j++)
				binaryGreedyMeshing(&vertices[i], value[j], j, key, 0);
}

static void	constructYAxisMesh(std::vector<DATA_TYPE> (&vertices)[6], uint64_t (&yAxisBitmask)[(CHUNK_WIDTH + 2) * (CHUNK_WIDTH + 2)], ChunkData &chunk, ChunkData *neightboursChunks[6])
{
	// Get the Y axis neighbours data
	for (uint64_t i = 0; i < CHUNK_WIDTH * CHUNK_WIDTH; i++) {
		uint64_t	x = i % CHUNK_WIDTH;
		uint64_t	z = i / CHUNK_WIDTH;
		if (isNeightbourLoaded(neightboursChunks[2]) && BLOCK_AT(neightboursChunks[2]->chunk, x, CHUNK_HEIGHT - 1, z))
			yAxisBitmask[i] |= (uint64_t)0x1 << 0;
		if (isNeightbourLoaded(neightboursChunks[3]) && BLOCK_AT(neightboursChunks[3]->chunk, x, 0, z))
			yAxisBitmask[i] |= (uint64_t)0x1 << (CHUNK_HEIGHT + 1);
	}

	std::map<uint8_t, uint32_t[CHUNK_WIDTH][CHUNK_HEIGHT]>	binaryPlaneHM[2];
	for (int i = 0; i < CHUNK_WIDTH * CHUNK_WIDTH; i++) {
		// Culling facing forwardX
		uint64_t	forwardYCol = yAxisBitmask[i] & ~(yAxisBitmask[i] >> 1);
		forwardYCol = forwardYCol >> 1;
		forwardYCol = forwardYCol & ~((uint64_t)1 << CHUNK_WIDTH);

		// Culling facing backwardX
		uint64_t	backwardYCol = yAxisBitmask[i] & ~(yAxisBitmask[i] << 1);
		backwardYCol = backwardYCol >> 1;
		backwardYCol = backwardYCol & ~((uint64_t)1 << CHUNK_WIDTH);

		for (int j = 0; j < CHUNK_WIDTH; j++) {
			uint8_t	id = BLOCK_AT(chunk.chunk, i % CHUNK_WIDTH, j, i / CHUNK_WIDTH);

			if (1 & (backwardYCol >> j))
				binaryPlaneHM[0][id][j][i / CHUNK_WIDTH] |= (uint64_t)0x1 << (i % CHUNK_WIDTH);

			if (1 & (forwardYCol >> j))
				binaryPlaneHM[1][id][j][i / CHUNK_WIDTH] |= (uint64_t)0x1 << (i % CHUNK_WIDTH);
		}
	}
	// Execute the binary greedy meshing algorythme for the X axis
	for (int i = 0; i < 2; i++)
		for (auto &[key, value] : binaryPlaneHM[i])
			for (int j = 0; j < CHUNK_WIDTH; j++)
			binaryGreedyMeshing(&vertices[i + 2], value[j], j, key, 1);
}

static void	constructZAxisMesh(std::vector<DATA_TYPE> (&vertices)[6], uint64_t (&zAxisBitmask)[(CHUNK_WIDTH + 2) * (CHUNK_WIDTH + 2)], ChunkData &chunk, ChunkData *neightboursChunks[6])
{
	// Get the Z axis neighbours data
	for (uint64_t i = 0; i < CHUNK_HEIGHT * CHUNK_WIDTH; i++) {
		uint64_t	x = i % CHUNK_WIDTH;
		uint64_t	y = i / CHUNK_HEIGHT;
		if (isNeightbourLoaded(neightboursChunks[0]) && BLOCK_AT(neightboursChunks[0]->chunk, x, y, CHUNK_WIDTH - 1))
			zAxisBitmask[i] |= (uint64_t)0x1 << 0;
		if (isNeightbourLoaded(neightboursChunks[1]) && BLOCK_AT(neightboursChunks[1]->chunk, x, y, 0))
			zAxisBitmask[i] |= (uint64_t)0x1 << (CHUNK_WIDTH + 1);
	}

	std::map<uint8_t, uint32_t[CHUNK_WIDTH][CHUNK_HEIGHT]>	binaryPlaneHM[2];
	for (int i = 0; i < CHUNK_WIDTH * CHUNK_WIDTH; i++) {
		// Culling facing forwardX
		uint64_t	forwardZCol = zAxisBitmask[i] & ~(zAxisBitmask[i] >> 1);
		forwardZCol = forwardZCol >> 1;
		forwardZCol = forwardZCol & ~((uint64_t)1 << CHUNK_WIDTH);

		// Culling facing backwardX
		uint64_t	backwardZCol = zAxisBitmask[i] & ~(zAxisBitmask[i] << 1);
		backwardZCol = backwardZCol >> 1;
		backwardZCol = backwardZCol & ~((uint64_t)1 << CHUNK_WIDTH);

		for (int j = 0; j < CHUNK_WIDTH; j++) {
			uint8_t	id = BLOCK_AT(chunk.chunk, i % CHUNK_WIDTH, i / CHUNK_WIDTH, j);

			if (1 & (backwardZCol >> j))
				binaryPlaneHM[0][id][j][i / CHUNK_WIDTH] |= (uint64_t)0x1 << (i % CHUNK_WIDTH);

			if (1 & (forwardZCol >> j))
				binaryPlaneHM[1][id][j][i / CHUNK_WIDTH] |= (uint64_t)0x1 << (i % CHUNK_WIDTH);
		}
	}
	// Execute the binary greedy meshing algorythme for the X axis
	for (int i = 0; i < 2; i++)
		for (auto &[key, value] : binaryPlaneHM[i])
			for (int j = 0; j < CHUNK_WIDTH; j++)
				binaryGreedyMeshing(&vertices[i + 4], value[j], j, key, 2);
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

	vector<DATA_TYPE>	vertices[6];

	constructXAxisMesh(vertices, xAxisBitmask, chunk, neightboursChunks);
	constructYAxisMesh(vertices, yAxisBitmask, chunk, neightboursChunks);
	constructZAxisMesh(vertices, zAxisBitmask, chunk, neightboursChunks);

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
