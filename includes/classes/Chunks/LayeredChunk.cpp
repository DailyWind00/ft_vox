/// class idependant system includes
# include <iostream>

# include "LayeredChunk.hpp"
# include "Noise.hpp"

// Destructor declation for the chunk layer interface
AChunkLayer::~AChunkLayer() {}

// LayeredChunk implementation
LayeredChunk::LayeredChunk(const uint8_t &id)
{
	// Chunk Layer allocation
	this->_layer = new AChunkLayer*[CHUNK_HEIGHT];
	for (int i = 0; i < CHUNK_HEIGHT; i++) {
		this->_layer[i] = new SingleBlockChunkLayer(id);
	}
}

void	LayeredChunk::generate(const glm::ivec3 &pos)
{
	// Pre-compute the perlin noise factors
	float	*factors = new float[CHUNK_WIDTH * CHUNK_WIDTH];

	for (int i = 0; i < CHUNK_WIDTH * CHUNK_WIDTH; i++) {
		float	factor = 0;
		float	amp = 16;
		
		for (int j = 0; j < 4; j++) {
			factor += Noise::perlin2D(glm::vec2{(i % CHUNK_WIDTH) / amp,
					((float)i / CHUNK_WIDTH) / amp}) * amp;
			amp -= 4;
		}
		factors[i] = factor;
	}

	// Populate the chunk according to the pre-computed perlin noise factors
	uint8_t	fstBlkPerLayer[CHUNK_HEIGHT] = {0};
	
	Noise::setSeed(2476858476);
	for (int i = pos.x; i < CHUNK_WIDTH + pos.x; i++) {
		for (int j = pos.z; j < CHUNK_WIDTH + pos.z; j++) {
			int	idx = (i - pos.x) * CHUNK_WIDTH + (j - pos.z);

			for (int k = pos.y; k < CHUNK_HEIGHT + pos.y; k++) {
				uint8_t	id = (k < factors[idx]);

				if (id != fstBlkPerLayer[k - pos.y] && i != pos.x && j != pos.z
						&& dynamic_cast<SingleBlockChunkLayer *>(this->_layer[k - pos.y]))
					this->_layer[k - pos.y] = _blockToLayer(this->_layer[k - pos.y]);
				this->_layer[k - pos.y]->setData(idx, id);
				if (i == pos.x && j == pos.z)
					fstBlkPerLayer[k - pos.y] = id;
			}
		}
	}
	delete [] factors;
}

void	LayeredChunk::print()
{
	for (int i = 0; i < CHUNK_HEIGHT; i++) {
		for (int j = 0; j < CHUNK_WIDTH * CHUNK_WIDTH ; j++) {
			if (dynamic_cast<SingleBlockChunkLayer *>(this->_layer[i])) {
				if (this->_layer[i]->getData(0) == 1)
					std::cout << "#";
				else 
					std::cout << ".";
				break;
			}
			if (!(j % CHUNK_WIDTH)) std::cout << "\n";
			if (this->_layer[i]->getData(j) == 1)
				std::cout << "# ";
			else 
				std::cout << ". ";
		}
		std::cout << "\n---------------------------------------------------------------------------------------------------" << std::endl;
	}
}

LayeredChunk::~LayeredChunk()
{
	for (int i = 0; i < CHUNK_HEIGHT; i++)
		delete this->_layer[i];
	delete [] this->_layer;
}

ChunkLayer		* LayeredChunk::_blockToLayer(AChunkLayer *layer)
{
	uint8_t	id = layer->getData(0);
	delete layer;

	return (new ChunkLayer(id));
}

SingleBlockChunkLayer	* LayeredChunk::_layerToBlock(AChunkLayer *layer)
{
	uint8_t	id = layer->getData(0);
	delete layer;
	
	return (new SingleBlockChunkLayer(id));
}
