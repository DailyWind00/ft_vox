/// Class idependant system includes

# include "ChunkHandler.hpp"

//// ChunkHandler class

/// public methods
AChunk	* ChunkHandler::createChunk(const glm::ivec3 &chunkPos)
{
	glm::ivec3	wordPos = {
		chunkPos.x * CHUNK_WIDTH,
		chunkPos.y * CHUNK_HEIGHT,
		chunkPos.z * CHUNK_WIDTH,
	};

	AChunk	* chunk = new LayeredChunk(1);
	chunk->generate(wordPos);
	
	size_t	count = 0;

	for (int i = 0; i < CHUNK_HEIGHT; i++)
		if (IS_LAYER_COMPRESSED(chunk, i))
			count++;

	if (count == CHUNK_HEIGHT)
		chunk = _chunkToBlock(chunk);
	return (chunk);
}
/// ---

/// private methods
AChunk	* ChunkHandler::_chunkToBlock(AChunk *chunk)
{
	uint8_t	id = BLOCK_AT(chunk, 0, 0, 0);

	delete chunk;

	return (new SingleBlockChunk(id));
}

AChunk	* ChunkHandler::_blockToChunk(AChunk *chunk)
{
	uint8_t	id = BLOCK_AT(chunk, 0, 0, 0);

	delete chunk;

	return (new LayeredChunk(id));
}
/// ---

//// ---
