# pragma once

/// Defines

/// System includes
# include <cstdint>
# include <cstring>

/// Dependencies
# include "AChunk.hpp"

/// Global variables

// Chunk layer interface.
class	AChunkLayer {
	public:
		AChunkLayer();
		virtual ~AChunkLayer() = 0;

		virtual uint8_t	& operator[](const size_t &i) = 0;

		virtual void	print() = 0;
};

// Chunk layer when every blocks in the layer are the same.
// Only contains a single uint8_t to represent the block id of the whole layer.
class	SingleBlockChunkLayer : public AChunkLayer {
	private:
		uint8_t	_id;

	public:
		SingleBlockChunkLayer(const uint8_t &id);
		~SingleBlockChunkLayer();

		// Allways return the block id of the layered no matter the value i.
		// This avoid compatibility issues with other type of layers
		uint8_t	& operator[](const size_t &i);

		void	print();
};

// Default chunk layer.
// Stores an array of bytes that are indices to the chunk block palette.
class	ChunkLayer : public AChunkLayer {
	private:
		uint8_t	*_data;

	public:
		ChunkLayer(const uint8_t &id);
		~ChunkLayer();
	
		uint8_t	& operator[](const size_t &i);

		void	print();
};

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

		void	generate(const glm::ivec3 &pos);

		AChunkLayer *	& operator[](const size_t &i);

		// Debugging method. Will not be used in the final release.
		void	print();
};

class	SingleBlockChunk : public AChunk {
	private:
		// This member will only be intenciated has a "SingleBlockChunkLayer".
		// An abstract type is used for compatibility with other chunk types.
		AChunkLayer	*_id;
	
	public:
		SingleBlockChunk(const uint8_t &id);
		~SingleBlockChunk();

		AChunkLayer *	& operator[](const size_t &i);
		
		void	print();
		void	generate(const glm::ivec3 &pos);
};
