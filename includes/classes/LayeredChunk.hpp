# pragma once

/// System includes
# include <cstdint>

/// Dependencies
# include "AChunk.hpp"

// Chunk layer interface.
typedef struct AChunkLayer {
	virtual ~AChunkLayer() = 0;
	//-virtual uint8_t	& operator[](const size_t &idx);
} AChunkLayer;

// Chunk layer when every blocks in the layer are the same.
typedef struct SingleBlockChunkLayer : public AChunkLayer {
	class Block	* block;
}	SingleBlockChunkLayer;

// Default chunk layer.
// Stores an array of bytes that are indices to the chunk block palette.
typedef struct ChunkLayer : public AChunkLayer {
	uint8_t	& operator[](const size_t &idx) {
		return (this->_data[idx]);
	}
	private:
		uint8_t	_data[CHUNK_WIDTH * CHUNK_WIDTH];
}	ChunkLayer;

// Default type of chunk.
// Contain a abstract type that stores a layer of block.
class	LayeredChunk : public AChunk {
	private:
		AChunkLayer	* _layer;

	public:
		LayeredChunk();
		~LayeredChunk();
};
