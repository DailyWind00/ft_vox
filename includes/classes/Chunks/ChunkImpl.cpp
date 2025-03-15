/// class idependant system includes
# include <iostream>

# include "ChunkImpl.hpp"
# include "Noise.hpp"

//// LayeredChunk class

/// Constructors & Destructors
LayeredChunk::LayeredChunk(const uint8_t &id)
{
	// Chunk Layer allocation
	this->_layer = new AChunkLayer*[CHUNK_HEIGHT];
	for (int i = 0; i < CHUNK_HEIGHT; i++)
		this->_layer[i] = new SingleBlockChunkLayer(id);
}

LayeredChunk::~LayeredChunk()
{
	for (int i = 0; i < CHUNK_HEIGHT; i++)
		delete this->_layer[i];
	delete [] this->_layer;
}
/// ---

/// Operator Overloads
AChunkLayer *	& LayeredChunk::operator[](const size_t &i)
{
	return (this->_layer[i]);
}
/// ---

/// Private Methods

float *	LayeredChunk::_computeHeatMap(const glm::ivec3 &pos)
{
	float *	factors = new float[CHUNK_WIDTH * CHUNK_WIDTH];
	
	for (int i = 0; i < CHUNK_WIDTH * CHUNK_WIDTH; i++) {
		float	factor = 0;
		float	amp = 64.0f;
	
		for (int j = 0; j < 8; j++) {	
			factor += Noise::perlin2D(glm::vec2{(2048.0f + pos.x + (i % CHUNK_WIDTH)) / (amp * 32.0f),
					(2048.0f + pos.z+ ((float)i / CHUNK_WIDTH)) / (amp * 32.0f)}) * amp;
			amp -= amp / 2.0f;
		}
		factors[i] = factor;
	}
	return (factors);
}

float *	LayeredChunk::_computeHumidityMap(const glm::ivec3 &pos)
{
	float *	factors = new float[CHUNK_WIDTH * CHUNK_WIDTH];

	for (int i = 0; i < pow(CHUNK_WIDTH, 2); i++) {
		float	factor = 0;
		float	amp = 64.0f;
	
		for (int j = 0; j < 8; j++) {	
			factor += Noise::perlin2D(glm::vec2{(1024.0f + pos.x + (i % CHUNK_WIDTH)) / (amp * 32.0f),
					(1024.0f + pos.z+ ((float)i / CHUNK_WIDTH)) / (amp * 32.0f)}) * amp;
			amp -= amp / 2.0f;
		}
		factors[i] = factor;
	}
	return (factors);
}

float *	LayeredChunk::_computeHeightMap(const glm::ivec3 &pos)
{
	// Pre-compute the perlin noise factors
	float		*factors = new float[CHUNK_WIDTH * CHUNK_WIDTH];
	uint32_t	maxPos = MAX_WORLD_SIZE * CHUNK_WIDTH;

	for (int i = 0; i < CHUNK_WIDTH * CHUNK_WIDTH; i++) {
		float	factor = 0;
		float	amp = 128;
		
		for (int j = 0; j < 10; j++) {
			factor += Noise::perlin2D(glm::vec2{(pos.x + maxPos + (i % CHUNK_WIDTH)) / (amp * 3),
					(pos.z + maxPos + ((float)i / CHUNK_WIDTH)) / (amp * 3)}) * (amp / 2);

			if (factor > 0)
				factor = pow(factor, 1.03);
			else
				factor = -pow(fabsf(factor), 1.04);

			amp -= amp / 2;
		}
		if (factor > 0)
			factor = pow(factor, 1.1);
		else
			factor = factor * 0.2;

		factor += Noise::perlin2D(glm::vec2{(pos.x + (i % CHUNK_WIDTH)) / 2048,
				(pos.z+ ((float)i / CHUNK_WIDTH)) / 2048}) * 512;

		factors[i] = factor;
	}
	return (factors);
}

float **	LayeredChunk::_computeCaveNoise(const glm::ivec3 &pos, float *heightMap)
{
	float		**factors = new float*[CHUNK_WIDTH * CHUNK_WIDTH];
	uint32_t	maxPos = MAX_WORLD_SIZE * CHUNK_WIDTH;
	
	for (int i = 0; i < CHUNK_WIDTH * CHUNK_WIDTH; i++) {
		float	factor = 0;

		factors[i] = new float[CHUNK_WIDTH];
		for (int j = 0; j < CHUNK_WIDTH; j++) {
			float	amp = 20.0f;

			if (j + pos.y > heightMap[0]) {
				factors[i][j] = 0;
				continue ;
			}

			float	x = (maxPos + pos.x + (i % CHUNK_WIDTH));
			float	y = (maxPos + pos.y + j);
			float	z = (maxPos + pos.z + ((float)i / CHUNK_WIDTH)) ;

			factor = Noise::perlin3D({x / amp, y / amp, z / amp}) * amp;
			amp = 120;
			factor -= fabsf(Noise::perlin3D({x / amp, y / amp, z / amp}) * amp);
			factors[i][j] = factor;
		}
	}
	return (factors);
}

uint8_t	LayeredChunk::_getBiomeID(const int &idx, const float *heatFactors, const float *wetFactors)
{
	int	heatLvl = TEMPERATE;
	int	wetLvl = MOIST;

	if (heatFactors[idx] < -9.0f)
		heatLvl = COLD;
	else if (heatFactors[idx] >= -9.0f && heatFactors[idx] <= 9.0f)
		heatLvl = TEMPERATE;
	else if (heatFactors[idx] > 9.0f)
		heatLvl = HOT;

	if (wetFactors[idx] < -9.0f)
		wetLvl = DRY;
	else if (wetFactors[idx] >= -9.0f && wetFactors[idx] <= 9.0f)
		wetLvl = MOIST;
	else if (wetFactors[idx] > 9.0f)
		wetLvl = DRENCHED;

	if (heatLvl == TEMPERATE && wetLvl == MOIST)
		return (PLAIN);
	else if (heatLvl == HOT && wetLvl == DRY)
		return (DESERT);
	else if (heatLvl == COLD && wetLvl == DRENCHED)
		return (FOREST);
	return (NONE);
}

uint8_t	LayeredChunk::_getBlockFromBiome(const uint8_t &y, const uint8_t &biomeID)
{
	uint8_t	blockID = 0;

	switch(biomeID) {
		case PLAIN:
			blockID = 1;
			break ;
		case DESERT:
			blockID = 2;
			break ;
		case FOREST:
			blockID = 3;
			break ;
		default:
			blockID = 120;
			break ;
	}
	return (blockID);
}

/// ---

/// Public methods
void	LayeredChunk::generate(const glm::ivec3 &pos)
{
	float *	heightFactors = _computeHeightMap(pos);
	float *	heatFactors = _computeHeatMap(pos);
	float *	humidityFactors = _computeHumidityMap(pos);
	float **	caveFactors = _computeCaveNoise(pos, heightFactors);

	// Store the first block of each layer to handle decompression
	uint8_t	fstBlkPerLayer[CHUNK_HEIGHT] = {0};

	for (int i = pos.y; i < CHUNK_HEIGHT + pos.y; i++)
		fstBlkPerLayer[i - pos.y] = (i < heightFactors[0]);
	
	// Populate the chunk according to the pre-computed perlin noise factors
	for (int i = pos.x; i < CHUNK_WIDTH + pos.x; i++) {
		for (int j = pos.z; j < CHUNK_WIDTH + pos.z; j++) {
			int	idx = (i - pos.x) * CHUNK_WIDTH + (j - pos.z);

			uint8_t	biomeID = _getBiomeID(idx, heatFactors, humidityFactors);

			for (int k = pos.y; k < CHUNK_HEIGHT + pos.y; k++) {
				uint8_t	id = _getBlockFromBiome(k, biomeID) * (k < heightFactors[idx] && caveFactors[idx][k - pos.y] < 0.01f);

				if (id != fstBlkPerLayer[k - pos.y] && dynamic_cast<SingleBlockChunkLayer *>(this->_layer[k - pos.y]))
					this->_layer[k - pos.y] = _blockToLayer(this->_layer[k - pos.y]);
				(*this->_layer[k - pos.y])[idx] = id;
			}
		}
	}
	delete [] heightFactors;
	delete [] heatFactors;
	delete [] humidityFactors;
}

void	LayeredChunk::print()
{
	for (int i = 0; i < CHUNK_HEIGHT; i++) {
		this->_layer[i]->print();
		std::cout << "\n---------------------------------------------------------------------------------------------------" << std::endl;
	}
}

/// ---

/// Private methods
ChunkLayer		* LayeredChunk::_blockToLayer(AChunkLayer *layer)
{
	uint8_t	id = (*layer)[0];
	delete layer;

	return (new ChunkLayer(id));
}

SingleBlockChunkLayer	* LayeredChunk::_layerToBlock(AChunkLayer *layer)
{
	uint8_t	id = (*layer)[0];
	delete layer;
	
	return (new SingleBlockChunkLayer(id));
}
/// ---

//// ---

//// SingleBlockChunk class

/// Constructors & Destructors
SingleBlockChunk::SingleBlockChunk(const uint8_t &id) : _id(new SingleBlockChunkLayer(id)) {}

SingleBlockChunk::~SingleBlockChunk()
{
	delete this->_id;
}
/// ---

/// Operator Overloads
AChunkLayer *	& SingleBlockChunk::operator[](const size_t &i)
{
	(void)i;
	return (this->_id);
}
/// ---

/// public methods
void	SingleBlockChunk::print()
{
	this->_id->print();
}

void	SingleBlockChunk::generate(const glm::ivec3 &pos) { (void)pos; }

/// ---

//// ---

//// AChunkLayer class

/// Constructors & Destructors
AChunkLayer:: AChunkLayer() {}
AChunkLayer::~AChunkLayer() {}
/// ---

//// SingleBlockChunkLayer class

/// Constructors & Destructors
SingleBlockChunkLayer::SingleBlockChunkLayer(const uint8_t &id) : _id(id) {}
SingleBlockChunkLayer::~SingleBlockChunkLayer() {}
/// ---

/// Operator Overloads
uint8_t	& SingleBlockChunkLayer::operator[](const size_t &i)
{
	(void)i;
	return (this->_id);
}
/// ---

/// public methods
void	SingleBlockChunkLayer::print()
{
	if (this->_id)
		std::cout << " #" << std::endl;
	else std::cout << " ." << std::endl;
}
/// ---

//// ---

//// ChunkLayer class

/// Constructors & Destructors
ChunkLayer::ChunkLayer(const uint8_t &id) : _data(new uint8_t[CHUNK_WIDTH * CHUNK_WIDTH])
{
	memset(this->_data, id, CHUNK_WIDTH * CHUNK_WIDTH);
}
ChunkLayer::~ChunkLayer()
{
	delete [] this->_data;
}
/// ---

/// Operator Overloads
uint8_t	& ChunkLayer::operator[](const size_t &i)
{
	return (this->_data[i]);
}
/// ---

/// public methods
void	ChunkLayer::print()
{
	for (int i = 0; i < CHUNK_WIDTH * CHUNK_WIDTH; i++) {
		if (this->_data[i])
			std::cout << " #";
		else std::cout << " .";
		if (!(i % CHUNK_WIDTH))
			std::cout << std::endl;
	}
}
/// ---

//// ---
