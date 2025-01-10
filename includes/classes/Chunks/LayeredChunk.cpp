/// class idependant system includes
# include <iostream>

# include "LayeredChunk.hpp"

// Destructor declation for the chunk layer interface
AChunkLayer::~AChunkLayer() {}

// LayeredChunk implementation
LayeredChunk::LayeredChunk()
{
	// Chunk Layer allocation
	this->_layer = new AChunkLayer*[CHUNK_HEIGHT];
	for (int i = 0; i < CHUNK_HEIGHT; i++)
		this->_layer[i] = new ChunkLayer;

	// Populating the chunk with random values
	srand(time(NULL));
	for (int i = 0; i < CHUNK_WIDTH * CHUNK_WIDTH; i++) {
		int	collumnHeight = rand() % 22;
		collumnHeight += 10;

		for (int j = 0; j < CHUNK_HEIGHT; j++) {
			if (j < collumnHeight) {
				this->_layer[j]->setData(i, 1);
				//-std::cout << " " << collumnHeight << " ";
			}
			else this->_layer[j]->setData(i, 0);
		}
		std::cout << '\n';
	}
}

void	LayeredChunk::print()
{
	for (int i = 0; i < CHUNK_HEIGHT; i++) {
		for (int j = 0; j < CHUNK_WIDTH * CHUNK_WIDTH ; j++) {
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
