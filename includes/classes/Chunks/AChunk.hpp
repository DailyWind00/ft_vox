# pragma once

/// Defines
# define CHUNK_WIDTH	32
# define CHUNK_HEIGHT	32

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
		virtual	~AChunk() = 0;
		virtual void	print() {}
		virtual void	generate(const glm::ivec3 &pos) { (void)pos; }
};
