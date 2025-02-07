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
		this->_layer[i] = new ChunkLayer(id);
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

/// Public methods
void	LayeredChunk::generate(const glm::ivec3 &pos)
{
	// Pre-compute the perlin noise factors
	float	*factors = new float[CHUNK_WIDTH * CHUNK_WIDTH];
	unsigned int	maxPos = MAX_WORLD_SIZE * CHUNK_WIDTH;

	for (int i = 0; i < CHUNK_WIDTH * CHUNK_WIDTH; i++) {
		float	factor = 0;
		float	amp = 256;
		
		for (int j = 0; j < 16; j++) {
			factor += Noise::perlin2D(glm::vec2{(pos.x + maxPos + (i % CHUNK_WIDTH)) / (amp * 3),
					(pos.z + maxPos + ((float)i / CHUNK_WIDTH)) / (amp * 3)}) * (amp / 2);
			amp -= 16;
		}
		factors[i] = factor;
	}

	// Populate the chunk according to the pre-computed perlin noise factors
	// uint8_t	fstBlkPerLayer[CHUNK_HEIGHT] = {0};
	
	for (int i = pos.x; i < CHUNK_WIDTH + pos.x; i++) {
		for (int j = pos.z; j < CHUNK_WIDTH + pos.z; j++) {
			int	idx = (i - pos.x) * CHUNK_WIDTH + (j - pos.z);

			for (int k = pos.y; k < CHUNK_HEIGHT + pos.y; k++) {
				uint8_t	id = (k < factors[idx]);

				//-if (id != fstBlkPerLayer[k - pos.y] && i != pos.x && j != pos.z
						//-&& dynamic_cast<SingleBlockChunkLayer *>(this->_layer[k - pos.y]))
					//-this->_layer[k - pos.y] = _blockToLayer(this->_layer[k - pos.y]);
				(*this->_layer[k - pos.y])[idx] = id;
				//-if (i == pos.x && j == pos.z)
					//-fstBlkPerLayer[k - pos.y] = id;
			}
		}
	}
	delete [] factors;
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
