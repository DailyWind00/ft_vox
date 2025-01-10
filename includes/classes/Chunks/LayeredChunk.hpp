# pragma once

/// System includes
# include <cstdint>
# include <iostream>

/// Dependencies
# include "AChunk.hpp"

// Chunk layer interface.
typedef struct AChunkLayer {
	AChunkLayer() {}
	virtual ~AChunkLayer() = 0;
	virtual	void setData(const size_t &i, const uint8_t &id) {
		std::cout << i << id << std::endl;
	}
	virtual	uint8_t getData(const size_t &i) {
		(void)i;
		return 0;
	}
} AChunkLayer;

// Chunk layer when every blocks in the layer are the same.
typedef struct SingleBlockChunkLayer : public AChunkLayer {
	class Block	* block;
}	SingleBlockChunkLayer;

// Default chunk layer.
// Stores an array of bytes that are indices to the chunk block palette.
typedef struct ChunkLayer : public AChunkLayer {
	ChunkLayer() : _data(new uint8_t[CHUNK_WIDTH * CHUNK_WIDTH]) {}
	~ChunkLayer() { delete [] this->_data; }
	void	setData(const size_t &i, const uint8_t &id) {
		if (i < CHUNK_WIDTH * CHUNK_WIDTH)
			this->_data[i] = id;
	}
	uint8_t	getData(const size_t &i) {
		if (i >= CHUNK_WIDTH * CHUNK_WIDTH)
			return (255);
		return (this->_data[i]);
	}
	private:
		uint8_t	*_data;
}	ChunkLayer;

// Default type of chunk.
// Contain a abstract type that stores a layer of block.
class	LayeredChunk : public AChunk {
	private:
		AChunkLayer	**_layer;

		void	print();

	public:
		LayeredChunk();
		~LayeredChunk();
};
