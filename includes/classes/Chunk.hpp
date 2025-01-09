# pragma once

/// Defines

/// System includes
# include <cstdlib>
# include <cstdint>

/// Dependencies
# include <glm/glm.hpp>

/// Global variables

typedef struct ChunkNode {
	ChunkNode() : _blockInfos(0), _n{NULL} {}
	
	ChunkNode	* operator[](const uint8_t &idx);
	uint16_t	_blockInfos;
	
	private:
		struct ChunkNode	*_n[8];
}	ChunkNode;

class Chunk {
	private:
		ChunkNode	*_node;
		uint8_t		_eMax;
	
	public:
		Chunk();
		Chunk(const uint16_t &filler);
		~Chunk();

		uint16_t	getBlockAt(const glm::uvec3 &pos);
		void		setBlockAt(const glm::uvec3 &pos);
};
