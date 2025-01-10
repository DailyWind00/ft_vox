/// class idependant system includes
# include <iostream>

# include "LayeredChunk.hpp"

// Destructor declation for the chunk layer interface
AChunkLayer::~AChunkLayer() {}

// LayeredChunk implementation
LayeredChunk::LayeredChunk()
{
	this->_layer = new ChunkLayer[CHUNK_HEIGHT];
}

LayeredChunk::~LayeredChunk()
{
	delete [] this->_layer;
}
