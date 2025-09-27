/// class idependant system includes
# include <iostream>
# include <mutex>

# include "ChunkImpl.hpp"
# include "Noise.hpp"
# include "features_declaration.h"

std::list<std::pair<glm::ivec3, WorldFeature> >	g_pendingFeatures;

std::mutex	g_pendingFeaturesMutex;

extern bool	NO_CAVES;

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
	
		for (int j = 0; j < 4; j++) {	
			factor += Noise::perlin2D(glm::vec2{(2048.0f + pos.x + (i % CHUNK_WIDTH)) / (amp * 32.0f),
					(2048.0f + pos.z+ ((float)i / CHUNK_WIDTH)) / (amp * 32.0f)}) * amp;
			amp -= amp / 2.0f;
		}
		int	r = rand() % 2;
		factors[i] = factor + r;
	}
	return (factors);
}

float *	LayeredChunk::_computeHumidityMap(const glm::ivec3 &pos)
{
	float *	factors = new float[CHUNK_WIDTH * CHUNK_WIDTH];

	for (int i = 0; i < pow(CHUNK_WIDTH, 2); i++) {
		float	factor = 0;
		float	amp = 64.0f;
	
		for (int j = 0; j < 4; j++) {	
			factor += Noise::perlin2D(glm::vec2{(1024.0f + pos.x + (i % CHUNK_WIDTH)) / (amp * 32.0f),
					(1024.0f + pos.z+ ((float)i / CHUNK_WIDTH)) / (amp * 32.0f)}) * amp;
			amp -= amp / 2.0f;
		}
		int	r = rand() % 2;
		factors[i] = factor + r;
	}
	return (factors);
}

float *	LayeredChunk::_computeFeatureMap(const glm::ivec3 &pos)
{
	float *	factors = new float[CHUNK_WIDTH * CHUNK_WIDTH];
	uint32_t	maxPos = MAX_WORLD_SIZE * CHUNK_WIDTH;

	for (int i = 0; i < pow(CHUNK_WIDTH, 2); i++) {
		float	factor = 0;


		factor += Noise::perlin2D(glm::vec2{(maxPos + pos.x + (i % CHUNK_WIDTH)) / 2.0f,
				(maxPos + pos.z+ ((float)i / CHUNK_WIDTH)) / 2.0f}) * 2.0f;
		if ((i % CHUNK_WIDTH) < 2 || (i % CHUNK_WIDTH) > 30 || (i / CHUNK_WIDTH) < 2 || (i / CHUNK_WIDTH) > 30) {
			factors[i] = 0;
			continue; 
		}
		factors[i] = pow(factor, 16.0f);
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

		factor += Noise::perlin2D(glm::vec2{(pos.x + (i % CHUNK_WIDTH)) / 8192,
				(pos.z+ ((float)i / CHUNK_WIDTH)) / 8192}) * 1024;

		factors[i] = factor;
	}
	return (factors);
}

float **	LayeredChunk::_computeCaveNoise(const glm::ivec3 &pos, float *heightMap)
{
	float **	factors = new float*[CHUNK_WIDTH * CHUNK_WIDTH];
	uint32_t	maxPos = MAX_WORLD_SIZE * CHUNK_WIDTH;
	
	for (int i = 0; i < (CHUNK_WIDTH * CHUNK_WIDTH); i+=1) {
		float	factor = 0;

		factors[i] = new float[CHUNK_WIDTH];
		for (int j = 0; j < CHUNK_WIDTH; j+=1) {
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
	int	wetLvl = DRENCHED;

	if (heatFactors[idx] < -9.0f)
		heatLvl = COLD;
	else if (heatFactors[idx] >= -9.0f && heatFactors[idx] <= 9.0f)
		heatLvl = TEMPERATE;
	else if (heatFactors[idx] > 9.0f)
		heatLvl = HOT;

	if (wetFactors[idx] < 0.0f)
		wetLvl = DRY;
	else if (wetFactors[idx] >= 0.0f)
		wetLvl = DRENCHED;

	if (heatLvl == COLD && wetLvl == DRY)
		return (SNOW_PLAIN);
	else if (heatLvl == COLD && wetLvl == DRENCHED)
		return (SNOW_FOREST);
	else if (heatLvl == TEMPERATE && wetLvl == DRY)
		return (PLAIN);
	else if (heatLvl == TEMPERATE && wetLvl == DRENCHED)
		return (FOREST);
	else if (heatLvl == HOT && wetLvl == DRY)
		return (DESERT);
	else if (heatLvl == HOT && wetLvl == DRENCHED)
		return (DESERT);
	return (NONE);
}

uint8_t	LayeredChunk::_getBlockFromBiome(const int &surface, const int &y, const uint8_t &biomeID)
{
	uint8_t	topLayerID = 0;
	uint8_t	soilLayerID = 0;
	uint8_t	stoneLayerID = 0;

	int	soilOffset = 1;
	int	stoneOffset = 4;

	switch(biomeID) {
		case PLAIN:
			topLayerID = 1;
			soilLayerID = 2;
			stoneLayerID = 3;
			
			soilOffset = 1;
			stoneOffset = 4;
			break ;
		case DESERT:
			topLayerID = 4;
			soilLayerID = 2;
			stoneLayerID = 3;
			
			soilOffset = 3;
			stoneOffset = 10;
			break ;
		case FOREST:
			topLayerID = 2;
			soilLayerID = 2;
			stoneLayerID = 3;
			
			soilOffset = 1;
			stoneOffset = 4;
			break ;
		case SNOW_PLAIN:
			topLayerID = 5;
			soilLayerID = 2;
			stoneLayerID = 3;

			soilOffset = 2;
			stoneOffset = 6;
			break ;
		case SNOW_FOREST:
			topLayerID = 5;
			soilLayerID = 2;
			stoneLayerID = 3;

			soilOffset = 2;
			stoneOffset = 6;
			break ;
		default:
			topLayerID = 120;
			soilLayerID = 120;
			stoneLayerID = 120;
			
			soilOffset = 1;
			stoneOffset = 4;
			break ;
	}

	uint8_t	blockID = topLayerID;

	if (y < surface - soilOffset && y >= surface - stoneOffset)
		blockID = soilLayerID;
	else if (y < surface - stoneOffset)
		blockID = stoneLayerID;

	return (blockID);
}

WorldFeature	LayeredChunk::_getFeatureFromBiome(const uint8_t &biomeID, const glm::ivec3 pos)
{
	int	variation;

	switch(biomeID) {
	case PLAIN:
		return (WorldFeature){pos, WF_NONE, NULL, true};
	case DESERT:
		variation = rand() % CACTIE_VARIATION_COUNT;
		return (WorldFeature){pos, WF_CACTUS, g_featureCactus[variation], true};
	case FOREST:
		variation = rand() % TREE_VARIATION_COUNT;
		return (WorldFeature){pos, WF_TREE, g_featureTree[variation], true};
	case SNOW_PLAIN:
		return (WorldFeature){pos, WF_NONE, NULL, true};
	case SNOW_FOREST:
		return (WorldFeature){pos, WF_SNOW_TREE, g_featureTree[0], true};
	default:
		return (WorldFeature){pos, WF_NONE, NULL, true};
	}
}

void	LayeredChunk::_handleWorldFeatureOverflow(std::pair<glm::ivec3, WorldFeature> wf, glm::ivec3 newDir, const bool reset)
{
	static glm::ivec3	dir = {0, 0, 0};
	WorldFeature		newFeature = wf.second;
	newFeature._origin = false;

	if (reset) {
		dir = {0, 0, 0};
		return ;
	}
	
	if (!wf.second._origin)
		return ;
	if (newDir.x != 0 && dir.x == 0) {
		newFeature._localPosition.z -= (CHUNK_WIDTH * newDir.z);
		g_pendingFeaturesMutex.lock();
		g_pendingFeatures.push_back(std::pair<glm::ivec3, WorldFeature>({wf.first.x, wf.first.y, wf.first.z + newDir.z}, newFeature));
		g_pendingFeaturesMutex.unlock();
		dir.z = newDir.z;
	}
	else if (newDir.y != 0 && dir.y == 0) {
		newFeature._localPosition.y -= (CHUNK_HEIGHT * newDir.y);
		g_pendingFeaturesMutex.lock();
		g_pendingFeatures.push_back(std::pair<glm::ivec3, WorldFeature>({wf.first.x, wf.first.y + newDir.y, wf.first.z}, newFeature));
		g_pendingFeaturesMutex.unlock();
		dir.y = newDir.y;
	}
	else if (newDir.z != 0 && dir.z == 0) {
		newFeature._localPosition.x -= (CHUNK_WIDTH * newDir.x);
		g_pendingFeaturesMutex.lock();
		g_pendingFeatures.push_back(std::pair<glm::ivec3, WorldFeature>({wf.first.x + newDir.x, wf.first.y, wf.first.z}, newFeature));
		g_pendingFeaturesMutex.unlock();
		dir.x = newDir.x;
	}
}

/// ---

/// Public methods
void	LayeredChunk::generate(const glm::ivec3 &pos)
{
	glm::ivec3	wPos = {pos.x / CHUNK_WIDTH, pos.y / CHUNK_HEIGHT, pos.z / CHUNK_WIDTH};

	std::list<std::pair<glm::ivec3, WorldFeature> >	localPendingFeatures;

	WorldNoises	noises = {
		_computeHeightMap(pos),
		_computeHeatMap(pos),
		_computeHumidityMap(pos),
		_computeFeatureMap(pos),

	};
	float **	caveFactors = (NO_CAVES) ? nullptr : _computeCaveNoise(pos, noises.heightMap);
	
	// Store the first block of each layer to handle decompression
	uint8_t	fstBlkPerLayer[CHUNK_HEIGHT] = {0};
	for (int i = pos.y; i < CHUNK_HEIGHT + pos.y; i++)
		fstBlkPerLayer[i - pos.y] = (i < noises.heightMap[0]);
	
	// Populate the chunk according to the pre-computed perlin noise factors
	for (int i = pos.x; i < CHUNK_WIDTH + pos.x; i++) {
		for (int j = pos.z; j < CHUNK_WIDTH + pos.z; j++) {
			// Convert's the 2D coordinates to a 1D index
			int	idx = (i - pos.x) * CHUNK_WIDTH + (j - pos.z);

			// Get the current biome ID
			uint8_t	biomeID = _getBiomeID(idx, noises.heatMap, noises.humidityMap);

			for (int k = pos.y; k < CHUNK_HEIGHT + pos.y; k++) {
				// Get the current blockID according to the pre-computed factors
				uint8_t	id = (NO_CAVES) ? _getBlockFromBiome(noises.heightMap[idx], k, biomeID) * ((k < noises.heightMap[idx])) : _getBlockFromBiome(noises.heightMap[idx], k, biomeID) * ((k < noises.heightMap[idx] && caveFactors[idx][k - pos.y] < 0.01f));

				if (k >= (int)noises.heightMap[idx] - 1 && k <= -1) {
					if (id == 0)
						id = 9;
					else if (id == 1 || id == 2)
						id = 4;
				}
				
				// Decompress the layer if needed
				if (id != fstBlkPerLayer[k - pos.y] && dynamic_cast<SingleBlockChunkLayer *>(this->_layer[k - pos.y]))
					this->_layer[k - pos.y] = _blockToLayer(this->_layer[k - pos.y]);

				// Set the block
				(*this->_layer[k - pos.y])[idx] = id;

				// Add new pending World features to be generated
				WorldFeature	newFeature = _getFeatureFromBiome(biomeID, {i - pos.x, k - pos.y, j - pos.z});
				if (k == (int)noises.heightMap[idx] && noises.featuresMap[idx] > WORLDFEATURE_THRESHOLDS[newFeature._type] && id != 0 && k > 0) {
					localPendingFeatures.push_back(std::pair(wPos, newFeature));
				}
			}
		}
	}

	if (!NO_CAVES) {
		for (int i = 0; i < CHUNK_WIDTH * CHUNK_WIDTH; i++)
			delete [] caveFactors[i];
		delete [] caveFactors;
	}
	delete [] noises.featuresMap;
	delete [] noises.heatMap;
	delete [] noises.humidityMap;
	delete [] noises.heightMap;

	// Recover pending features from other bioms in global feature list
	g_pendingFeaturesMutex.lock();
	for (auto it = g_pendingFeatures.begin(); it != g_pendingFeatures.end();) {
		if (it->first == wPos && it->second._origin == false) {
			localPendingFeatures.push_back(*it);
			g_pendingFeatures.erase(it);
			it = g_pendingFeatures.begin();
		}
		else
			it++;
	}
	g_pendingFeaturesMutex.unlock();

	for (auto it = localPendingFeatures.begin(); it != localPendingFeatures.end();) {
		if (it->second._type == WF_NONE) {
			it++;
			continue ;
		}

		// Iterate over all the block of the current feature to copy them in the current chunk
		for (int i = 1; i <= it->second._data[0].x; i++) {
			// Convert's the 2D coordinates to a 1D index
			int		idx = (it->second._data[i].x + it->second._localPosition.x) * CHUNK_WIDTH + (it->second._data[i].z + it->second._localPosition.z);

			// Handle World features overflow in the Y axix
			if (it->second._localPosition.y + it->second._data[i].y >= CHUNK_HEIGHT) {
				_handleWorldFeatureOverflow(*it, {0, 1, 0}, false);
				continue ;
			}
			else if (it->second._localPosition.y + it->second._data[i].y < 0) {
				_handleWorldFeatureOverflow(*it, {0, -1, 0}, false);
				continue ;
			}
			
			// Handle World features overflow in the Z axix
			if (it->second._localPosition.z + it->second._data[i].z >= CHUNK_WIDTH) {
				_handleWorldFeatureOverflow(*it, {1, 0, 0}, false);
				continue ;
			}
			else if (it->second._localPosition.z + it->second._data[i].z < 0) {
				_handleWorldFeatureOverflow(*it, {-1, 0, 0}, false);
				continue ;
			}

			// Handle World features overflow in the X axix
			if (it->second._localPosition.x + it->second._data[i].x >= CHUNK_WIDTH) {
				_handleWorldFeatureOverflow(*it, {0, 0, 1}, false);
				continue ;
			}
			else if (it->second._localPosition.x + it->second._data[i].x < 0) {
				_handleWorldFeatureOverflow(*it, {0, 0, -1}, false);
				continue ;
			}

			// Decompress the layer if needed
			if (it->second._data[i].w != fstBlkPerLayer[it->second._localPosition.y + it->second._data[i].y] && dynamic_cast<SingleBlockChunkLayer *>(this->_layer[it->second._localPosition.y + it->second._data[i].y]))
				this->_layer[it->second._localPosition.y + it->second._data[i].y] = _blockToLayer(this->_layer[it->second._localPosition.y + it->second._data[i].y]);
			
			// Set the block
			(*this->_layer[it->second._localPosition.y + it->second._data[i].y])[idx] = it->second._data[i].w;
		}
		// Reset the function static variable for future usage
		_handleWorldFeatureOverflow(*it, {0, 0, 0}, true);

		// Remove the pending feature from the list
		localPendingFeatures.erase(it);
		it = localPendingFeatures.begin();
	}
}

void	LayeredChunk::print()
{
	for (int i = 0; i < CHUNK_HEIGHT; i++) {
		this->_layer[i]->print();
		std::cout << "\n---------------------------------------------------------------------------------------------------" << std::endl;
	}
}

void	LayeredChunk::setBlock(const glm::ivec3 &pos, const uint8_t &blockID)
{
	uint32_t	idx = pos.x * CHUNK_WIDTH + pos.z;

	if (dynamic_cast<SingleBlockChunkLayer *>(this->_layer[pos.y]))
		this->_layer[pos.y] = _blockToLayer(this->_layer[pos.y]);
	(*this->_layer[pos.y])[idx] = blockID;
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

void	SingleBlockChunk::setBlock(const glm::ivec3 &pos, const uint8_t &blockID) { (void)pos; (void)blockID; }

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
