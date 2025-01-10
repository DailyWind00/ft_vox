# pragma once

/// Defines
# define CHUNK_WIDTH	32
# define CHUNK_HEIGHT	32

/// System includes
# include <cstdlib>
# include <cstdint>
# include <vector>

/// Dependencies

/// Global variables

class	AChunk {
	protected:
		// holds uint for but will be replaced by Block class
		std::vector<uint8_t>	_blockPalette;

	public:
		AChunk();
		virtual	~AChunk() = 0;
};
