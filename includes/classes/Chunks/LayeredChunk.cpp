/// class idependant system includes
# include <iostream>

# include "LayeredChunk.hpp"
# include "Noise.hpp"

// Destructor declation for the chunk layer interface
AChunkLayer::~AChunkLayer() {}

// LayeredChunk implementation
LayeredChunk::LayeredChunk()
{
	// Chunk Layer allocation
	this->_layer = new AChunkLayer*[CHUNK_HEIGHT];
	for (int i = 0; i < CHUNK_HEIGHT; i++)
		this->_layer[i] = new ChunkLayer;

	// setting Perlin Noise seed
	Noise::setSeed(2476858476);
	unsigned int	randFactor = 1 + rand();

	for (int i = 0; i < CHUNK_WIDTH * CHUNK_WIDTH; i++) {
		float	factor = 10 + Noise::perlin2D(glm::vec2{(i % CHUNK_WIDTH) * 0.1, (i / CHUNK_WIDTH) * 0.1}, randFactor) * 10;

		for (int j = 0; j < CHUNK_HEIGHT; j++) {
			if (j < factor) {
				this->_layer[j]->setData(i, 1);
			}
			else this->_layer[j]->setData(i, 0);
		}
	}

	for (int i = 0; i < CHUNK_HEIGHT; i++) {
		size_t	consecutiveBlocks = 0;
		uint8_t	firstBlock = this->_layer[i]->getData(0);

		for (int j = 1; j < CHUNK_WIDTH * CHUNK_WIDTH; j++) {
			if (this->_layer[i]->getData(j) != firstBlock)
				break ;
			consecutiveBlocks++;
		}
		if (consecutiveBlocks == (CHUNK_WIDTH * CHUNK_WIDTH) - 1)
			this->_layer[i] = _layerToBlock(this->_layer[i]);
	}
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
				std::cout << "#";
			else 
				std::cout << ".";
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

SingleBlockChunkLayer	* LayeredChunk::_layerToBlock(AChunkLayer *layer)
{
	uint8_t	id = layer->getData(0);
	delete layer;
	
	SingleBlockChunkLayer	*block = new SingleBlockChunkLayer();
	block->setData(0, id);

	return (block);
}
