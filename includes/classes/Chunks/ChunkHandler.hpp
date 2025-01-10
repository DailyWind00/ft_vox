# pragma once

/// Defines

/// System includes
# include <cstdlib>
# include <unordered_map>

/// Dependencies
# include <glm/glm.hpp>

/// Global variables

class	ChunkHandler {
	private:
		std::unordered_map<glm::uvec3, class AChunk *>	chunks;
	
	public:
		;
};
