# pragma once

/// System includes
# include <cstdint>
# include <iostream>

/// Dependencies
# include "AChunk.hpp"
# include "glm/glm.hpp"

// Chunk layer interface.
typedef struct AChunkLayer {
	AChunkLayer() {}
	virtual ~AChunkLayer() = 0;

	virtual uint8_t	& operator[](const size_t &i) = 0;

} AChunkLayer;

// Chunk layer when every blocks in the layer are the same.
typedef struct SingleBlockChunkLayer : public AChunkLayer {
	SingleBlockChunkLayer(const uint8_t &id) : _id(id) {}

	uint8_t	& operator[](const size_t &i) {
		(void)i;
		return (this->_id);
	}
	
	private:
		uint8_t	_id;
}	SingleBlockChunkLayer;

// Default chunk layer.
// Stores an array of bytes that are indices to the chunk block palette.
typedef struct ChunkLayer : public AChunkLayer {

	ChunkLayer(const uint8_t &id) : _data(new uint8_t[CHUNK_WIDTH * CHUNK_WIDTH]) {
		for (size_t i = 0; i < CHUNK_WIDTH * CHUNK_WIDTH; i++)
			this->_data[i] = id;
	}
	~ChunkLayer() { delete [] this->_data; }

	uint8_t	& operator[](const size_t &i) {
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

		ChunkLayer		* _blockToLayer(AChunkLayer *layer);
		SingleBlockChunkLayer	* _layerToBlock(AChunkLayer *layer);
	public:
		LayeredChunk(const uint8_t &id);
		~LayeredChunk();

		AChunkLayer *	& operator[](const size_t &i);

		void	generate(const glm::ivec3 &pos);
		void	print();
};
