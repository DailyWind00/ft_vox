# pragma once

/// Defines

/// System includes
# include <cstdlib>

/// Dependencies
# include "glm/glm.hpp"
# include "chunk.h"

/// Global variables

class	ChunkHandler {
	private:
		static class AChunk	* _chunkToBlock(class AChunk *chunk);
		static class AChunk	* _blockToChunk(class AChunk *chunk);

	public:
		static class AChunk	* createChunk(const glm::ivec3 &chunkPos);
};
