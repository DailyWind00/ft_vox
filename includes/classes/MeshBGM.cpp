# include <VoxelSystem.hpp>

// Check if a neighbour chunk mesh is loaded
static inline bool	isNeightbourLoaded(ChunkData *neightbour) {
	return neightbour && neightbour->chunk && (neightbour->mesh || neightbour->inCreation);
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

static uint64_t		binMap(const uint64_t &x, const uint64_t &count, const uint64_t &stride)
{
	int	res = 0;

	for (uint64_t i = 0; i < count * stride; i++) {
		if (i % stride == 0)
			res |= (x & 0x01) << i;
		else
			res |= (~x & 0x01) << i;
	}
	return (res);
}

static DATA_TYPE	constructFaceVertice(const glm::ivec3 &pos, const glm::ivec2 &len, const glm::ivec2 &uv, const uint8_t &axis, const uint8_t &blockID) {
	DATA_TYPE	data = 0;

	data |= (pos.x & 0x3F);		// X
	data |= (pos.y & 0x3F) << 6;	// Y
	data |= (pos.z & 0x3F) << 12;	// Z
	data |= (axis & 0x07) << 18;	// face
	data |= (blockID & 0x1F) << 22;	// blockID
	data |= (uv.x & 0x01) << 27;	// UV X
	data |= (uv.y & 0x01) << 28;	// UV X
	data |= (len.x & (uint64_t)0x3F) << 32;
	data |= (len.y & (uint64_t)0x3F) << 38;

	return (data);
}

// Mesh Bitmask
// pos: 6 * 3 bits
// face: 3 bits
// id: 5 bits
static void	constructFace(vector<DATA_TYPE> *vertices, const glm::ivec3 &pos, const uint8_t &blockID, const glm::ivec2 &size, const uint8_t &axis)
{
	switch(axis) {
	case 0:	// X
		vertices->push_back(constructFaceVertice({pos.z, pos.y, pos.x}, {size.x, size.y}, {0, 0}, axis, blockID));
		vertices->push_back(constructFaceVertice({pos.z, pos.y + size.y, pos.x}, {size.x, size.y}, {0, 1}, axis, blockID));
		vertices->push_back(constructFaceVertice({pos.z + size.x, pos.y, pos.x}, {size.x, size.y}, {1, 0}, axis, blockID));

		vertices->push_back(constructFaceVertice({pos.z + size.x, pos.y + size.y, pos.x}, {size.x, size.y}, {1, 1}, axis, blockID));
		vertices->push_back(constructFaceVertice({pos.z + size.x, pos.y, pos.x}, {size.x, size.y}, {1, 0}, axis, blockID));
		vertices->push_back(constructFaceVertice({pos.z, pos.y + size.y, pos.x}, {size.x, size.y}, {0, 1}, axis, blockID));
		break ;
	
	case 1: // X
		vertices->push_back(constructFaceVertice({pos.z, pos.y, pos.x + 1}, {size.x, size.y}, {0, 0}, axis, blockID));
		vertices->push_back(constructFaceVertice({pos.z + size.x, pos.y, pos.x + 1}, {size.x, size.y}, {1, 0}, axis, blockID));
		vertices->push_back(constructFaceVertice({pos.z, pos.y + size.y, pos.x + 1}, {size.x, size.y}, {0, 1}, axis, blockID));

		vertices->push_back(constructFaceVertice({pos.z + size.x, pos.y + size.y, pos.x + 1}, {size.x, size.y}, {1, 1}, axis, blockID));
		vertices->push_back(constructFaceVertice({pos.z, pos.y + size.y, pos.x + 1}, {size.x, size.y}, {0, 1}, axis, blockID));
		vertices->push_back(constructFaceVertice({pos.z + size.x, pos.y, pos.x + 1}, {size.x, size.y}, {1, 0}, axis, blockID));
		break ;

	case 2:	// Y
		vertices->push_back(constructFaceVertice({pos.z, pos.y, pos.x}, {size.x, size.y}, {0, 0}, axis, blockID));
		vertices->push_back(constructFaceVertice({pos.z + size.x, pos.y, pos.x}, {size.x, size.y}, {1, 0}, axis, blockID));
		vertices->push_back(constructFaceVertice({pos.z, pos.y, pos.x + size.y}, {size.x, size.y}, {0, 1}, axis, blockID));

		vertices->push_back(constructFaceVertice({pos.z + size.x, pos.y, pos.x + size.y}, {size.x, size.y}, {1, 1}, axis, blockID));
		vertices->push_back(constructFaceVertice({pos.z, pos.y, pos.x + size.y}, {size.x, size.y}, {0, 1}, axis, blockID));
		vertices->push_back(constructFaceVertice({pos.z + size.x, pos.y, pos.x}, {size.x, size.y}, {1, 0}, axis, blockID));
		break ;
	
	case 3:	// Y
		vertices->push_back(constructFaceVertice({pos.z, pos.y + 1, pos.x}, {size.x, size.y}, {0, 0}, axis, blockID));
		vertices->push_back(constructFaceVertice({pos.z, pos.y + 1, pos.x + size.y}, {size.x, size.y}, {0, 1}, axis, blockID));
		vertices->push_back(constructFaceVertice({pos.z + size.x, pos.y + 1, pos.x}, {size.x, size.y}, {1, 0}, axis, blockID));

		vertices->push_back(constructFaceVertice({pos.z + size.x, pos.y + 1, pos.x + size.y}, {size.x, size.y}, {1, 1}, axis, blockID));
		vertices->push_back(constructFaceVertice({pos.z + size.x, pos.y + 1, pos.x}, {size.x, size.y}, {1, 0}, axis, blockID));
		vertices->push_back(constructFaceVertice({pos.z, pos.y + 1, pos.x + size.y}, {size.x, size.y}, {0, 1}, axis, blockID));
		break ;
	
	case 4:	// Z
		vertices->push_back(constructFaceVertice({pos.z, pos.y, pos.x}, {size.x, size.y}, {0, 0}, axis, blockID));
		vertices->push_back(constructFaceVertice({pos.z, pos.y, pos.x + size.x}, {size.x, size.y}, {1, 0}, axis, blockID));
		vertices->push_back(constructFaceVertice({pos.z, pos.y + size.y, pos.x}, {size.x, size.y}, {0, 1}, axis, blockID));

		vertices->push_back(constructFaceVertice({pos.z, pos.y + size.y, pos.x + size.x}, {size.x, size.y}, {1, 1}, axis, blockID));
		vertices->push_back(constructFaceVertice({pos.z, pos.y + size.y, pos.x}, {size.x, size.y}, {0, 1}, axis, blockID));
		vertices->push_back(constructFaceVertice({pos.z, pos.y, pos.x + size.x}, {size.x, size.y}, {1, 0}, axis, blockID));
		break ;
	
	case 5:	// Z
		vertices->push_back(constructFaceVertice({pos.z + 1, pos.y, pos.x}, {size.x, size.y}, {0, 0}, axis, blockID));
		vertices->push_back(constructFaceVertice({pos.z + 1, pos.y + size.y, pos.x}, {size.x, size.y}, {0, 1}, axis, blockID));
		vertices->push_back(constructFaceVertice({pos.z + 1, pos.y, pos.x + size.x}, {size.x, size.y}, {1, 0}, axis, blockID));

		vertices->push_back(constructFaceVertice({pos.z + 1, pos.y + size.y, pos.x + size.x}, {size.x, size.y}, {1, 1}, axis, blockID));
		vertices->push_back(constructFaceVertice({pos.z + 1, pos.y, pos.x + size.x}, {size.x, size.y}, {1, 0}, axis, blockID));
		vertices->push_back(constructFaceVertice({pos.z + 1, pos.y + size.y, pos.x}, {size.x, size.y}, {0, 1}, axis, blockID));
		break ;
	
	default:
		break ;
	}
}

// Binary greedy meshing algorythme
// Will quickly construte a mesh plane from a binary plane
static void	binaryGreedyMeshing(std::vector<DATA_TYPE> *vertices, uint32_t plane[CHUNK_WIDTH], const uint32_t &depth, const uint8_t &blockID, const uint8_t &axis, const uint8_t &LOD)
{
	for (int i = 0; i < CHUNK_WIDTH; i += LOD) {
		int	col = 0;

		while (col < CHUNK_WIDTH ) {
			col += trailing_zeros32(plane[i] >> col);
			if (col % LOD != 0)
				col += LOD - (col % LOD);
			if (col >= CHUNK_WIDTH)
				continue ;

			int	height = trailing_ones32(plane[i] >> col);
			if (height % LOD != 0)
				height += LOD - (height % LOD);

			while (1 & (plane[i] >> (col + height)) && col + height < CHUNK_WIDTH)
				height += LOD;

			int	width = LOD;

			uint32_t	h = binMap(0x1, height / LOD, LOD);
			uint32_t	heightMask = h << col;

			while (i + width < CHUNK_WIDTH) {
				uint32_t	next = (plane[i + width] >> col);

				if (h != next)
					break ;
				plane[i + width] = plane[i + width] & ~heightMask;
				width += LOD;
			}

			glm::ivec3	pos = {0, 0, 0};
			glm::ivec2	size = {0, 0};

			if (axis == 0 || axis == 1) {
				pos = {depth, i, col};
				size = {height, width};
			}
			else if (axis == 2 || axis == 3){
				pos = {col, depth, i};
				size = {width, height};
			}
			else if (axis == 4 || axis == 5) {
				pos = {col, i, depth};
				size = {height, width};
			}

			constructFace(vertices, pos, blockID, size, axis);
			col += height;
		}
	}
}

static bool	isVoxelTransparent(const uint8_t &id) {
	return (id == 0 || id == 9);
}

static void	constructXAxisMesh(std::vector<DATA_TYPE> *vertices, uint64_t (&xAxisBitmask)[(CHUNK_WIDTH + 2) * (CHUNK_WIDTH + 2)], ChunkData &chunk, ChunkData *neightboursChunks[6], const uint8_t &LOD)
{
	// Get the X axis neighbours data
	for (uint64_t i = 0; i < CHUNK_HEIGHT * CHUNK_WIDTH; i++) {
		uint64_t	y = i / CHUNK_WIDTH;
		uint64_t	z = i % CHUNK_WIDTH;

		// Temporary fix for hole in the map (NEED TO FIND A BETTER SOLUTION)
		// if (isNeightbourLoaded(neightboursChunks[4]) && BLOCK_AT(neightboursChunks[4]->chunk, CHUNK_WIDTH - 1, y, z))
		// 	xAxisBitmask[i] |= (uint64_t)0x1 << 0;
		if (isNeightbourLoaded(neightboursChunks[5]) && !isVoxelTransparent(BLOCK_AT(neightboursChunks[5]->chunk, 0, y, z)))
			xAxisBitmask[i] |= (uint64_t)0x1 << (CHUNK_WIDTH + 1);
	}
	
	std::map<uint8_t, uint32_t[CHUNK_WIDTH][CHUNK_WIDTH]>	binaryPlaneHM[2];
	for (int i = 0; i < CHUNK_WIDTH * CHUNK_WIDTH; i++) {
		// Culling facing forwardX
		uint64_t	forwardXCol = xAxisBitmask[i] & ~(xAxisBitmask[i] >> 1);
		forwardXCol = forwardXCol >> LOD;
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
			for (int j = 0; j < CHUNK_WIDTH; j += LOD)
				binaryGreedyMeshing(vertices, value[j], j, key, i, LOD);
}

static void	constructYAxisMesh(std::vector<DATA_TYPE> *vertices, uint64_t (&yAxisBitmask)[(CHUNK_WIDTH + 2) * (CHUNK_WIDTH + 2)], ChunkData &chunk, ChunkData *neightboursChunks[6], const uint8_t &LOD)
{
	// Get the Y axis neighbours data
	for (uint64_t i = 0; i < CHUNK_WIDTH * CHUNK_WIDTH; i++) {
		uint64_t	x = i % CHUNK_WIDTH;
		uint64_t	z = i / CHUNK_WIDTH;
		if (isNeightbourLoaded(neightboursChunks[2]) && !isVoxelTransparent(BLOCK_AT(neightboursChunks[2]->chunk, x, CHUNK_HEIGHT - 1, z)))
			yAxisBitmask[i] |= (uint64_t)0x1 << 0;
		if (isNeightbourLoaded(neightboursChunks[3]) && !isVoxelTransparent(BLOCK_AT(neightboursChunks[3]->chunk, x, 0, z)))
			yAxisBitmask[i] |= (uint64_t)0x1 << (CHUNK_HEIGHT + 1);
	}

	std::map<uint8_t, uint32_t[CHUNK_WIDTH][CHUNK_HEIGHT]>	binaryPlaneHM[2];
	for (int i = 0; i < CHUNK_WIDTH * CHUNK_WIDTH; i++) {
		// Culling facing forwardX
		uint64_t	forwardYCol = yAxisBitmask[i] & ~(yAxisBitmask[i] >> 1);
		forwardYCol = forwardYCol >> LOD;
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
			for (int j = 0; j < CHUNK_WIDTH; j += LOD)
				binaryGreedyMeshing(vertices, value[j], j, key, i + 2, LOD);
}

static void	constructWaterAxisMesh(std::vector<DATA_TYPE> *vertices, uint64_t (&yAxisBitmask)[(CHUNK_WIDTH + 2) * (CHUNK_WIDTH + 2)], ChunkData &chunk, ChunkData *neightboursChunks[6], const uint8_t &LOD)
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
		forwardYCol = forwardYCol >> LOD;
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
	for (auto &[key, value] : binaryPlaneHM[1])
		for (int j = 0; j < CHUNK_WIDTH; j += LOD)
			binaryGreedyMeshing(vertices, value[j], j, key, 3, LOD);
}

static void	constructZAxisMesh(std::vector<DATA_TYPE> *vertices, uint64_t (&zAxisBitmask)[(CHUNK_WIDTH + 2) * (CHUNK_WIDTH + 2)], ChunkData &chunk, ChunkData *neightboursChunks[6], const uint8_t &LOD)
{
	// Get the Z axis neighbours data
	for (uint64_t i = 0; i < CHUNK_HEIGHT * CHUNK_WIDTH; i++) {
		uint64_t	x = i % CHUNK_WIDTH;
		uint64_t	y = i / CHUNK_HEIGHT;
		if (isNeightbourLoaded(neightboursChunks[0]) && !isVoxelTransparent(BLOCK_AT(neightboursChunks[0]->chunk, x, y, CHUNK_WIDTH - 1)))
			zAxisBitmask[i] |= (uint64_t)0x1 << 0;
		if (isNeightbourLoaded(neightboursChunks[1]) && !isVoxelTransparent(BLOCK_AT(neightboursChunks[1]->chunk, x, y, 0)))
			zAxisBitmask[i] |= (uint64_t)0x1 << (CHUNK_WIDTH + 1);
	}

	std::map<uint8_t, uint32_t[CHUNK_WIDTH][CHUNK_HEIGHT]>	binaryPlaneHM[2];
	for (int i = 0; i < CHUNK_WIDTH * CHUNK_WIDTH; i++) {
		// Culling facing forwardX
		uint64_t	forwardZCol = zAxisBitmask[i] & ~(zAxisBitmask[i] >> 1);
		forwardZCol = forwardZCol >> LOD;
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
			for (int j = 0; j < CHUNK_WIDTH; j += LOD)
				binaryGreedyMeshing(vertices, value[j], j, key, i + 4, LOD);
}

void	VoxelSystem::_constructWaterMesh(std::vector<DATA_TYPE> *vertices, ChunkData &chunk, ChunkData *neightboursChunks[6], const uint8_t &LOD) {
	uint64_t	waterBitmask[(CHUNK_WIDTH + 2) * (CHUNK_HEIGHT + 2)] = {0};

	// Set the bitmasks of all the axis
	for (uint64_t y = 0; y < CHUNK_HEIGHT; y += LOD)
		for (uint64_t z = 0; z < CHUNK_WIDTH; z += LOD)
			for (uint64_t x = 0; x < CHUNK_WIDTH; x += LOD)
				if (BLOCK_AT(chunk.chunk, x, y, z) == 9)
					waterBitmask[z * CHUNK_WIDTH + x] |= binMap(0x1, LOD, 1) << (y + 1);	// Bit-shift is offset by one to allow for neighbour data
	
	constructWaterAxisMesh(vertices, waterBitmask, chunk, neightboursChunks, LOD);
}

void	VoxelSystem::_constructChunkMesh(std::vector<DATA_TYPE> *vertices, ChunkData &chunk, ChunkData *neightboursChunks[6], const uint8_t &LOD) {
	uint64_t	xAxisBitmask[(CHUNK_WIDTH + 2) * (CHUNK_HEIGHT + 2)] = {0};
	uint64_t	yAxisBitmask[(CHUNK_WIDTH + 2) * (CHUNK_WIDTH + 2)] = {0};
	uint64_t	zAxisBitmask[(CHUNK_WIDTH + 2) * (CHUNK_HEIGHT + 2)] = {0};

	// Set the bitmasks of all the axis
	for (uint64_t y = 0; y < CHUNK_HEIGHT; y += LOD) {
		for (uint64_t z = 0; z < CHUNK_WIDTH; z += LOD) {
			for (uint64_t x = 0; x < CHUNK_WIDTH; x += LOD) {
				// For each axis, write a bit to represent a solid block
				uint8_t	id = BLOCK_AT(chunk.chunk, x, y, z);
				if (id != 0 && id != 9) {
					xAxisBitmask[y * CHUNK_HEIGHT + z] |= binMap(0x1, LOD, 1) << (x + 1);	// Bit-shift is offset by one to allow for neighbour data
					yAxisBitmask[z * CHUNK_WIDTH + x] |= binMap(0x1, LOD, 1) << (y + 1);	// Bit-shift is offset by one to allow for neighbour data
					zAxisBitmask[y * CHUNK_HEIGHT + x] |= binMap(0x1, LOD, 1) << (z + 1);	// Bit-shift is offset by one to allow for neighbour data
				}
			}
		}
	}

	constructXAxisMesh(vertices, xAxisBitmask, chunk, neightboursChunks, LOD);
	constructYAxisMesh(vertices, yAxisBitmask, chunk, neightboursChunks, LOD);
	constructZAxisMesh(vertices, zAxisBitmask, chunk, neightboursChunks, LOD);

}
