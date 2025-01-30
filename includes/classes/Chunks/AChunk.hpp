# pragma once

/// Defines
# define CHUNK_WIDTH	32
# define CHUNK_HEIGHT	32
# define MAX_WORLD_SIZE	500

# define BLOCK_AT(chunk, x, y, z) (int)(*(*chunk)[y])[x * CHUNK_WIDTH + z] 
# define IS_LAYER_COMPRESSED(chunk, y)	dynamic_cast<SingleBlockChunkLayer *>((*chunk)[y])
# define IS_CHUNK_COMPRESSED(chunk)	dynamic_cast<SingleBlockChunk *>(chunk)

/// System includes
# include <cstdlib>
# include <cstdint>
# include <vector>

/// Dependencies
# include "glm/glm.hpp"

/// Global variables

class	AChunk {
	protected:
		// holds uint for but will be replaced by Block class
		std::vector<uint8_t>	_blockPalette;

	public:
		AChunk();
		AChunk(const uint8_t &id) { (void)id; }

		virtual class AChunkLayer *	& operator[](const size_t &i) = 0;

		virtual	~AChunk() = 0;
		virtual void	print() = 0;
		virtual void	generate(const glm::ivec3 &pos) = 0;
};
