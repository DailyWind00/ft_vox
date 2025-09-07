# pragma once

/// Defines
# define DATA_TYPE	uint64_t

/// System includes
# include <vector>

/// Dependencies
# include "BufferGL.hpp"
# include "Shader.hpp"

class	ChunkMesh {
	private:
		GLuint		_VAO = 0;
		BufferGL *	_VBO = nullptr;

		std::vector<DATA_TYPE>	_data;
		size_t		_dataSize = 0;

		bool	_toDelete = false;
	
	public:
		ChunkMesh(const std::vector<DATA_TYPE> &data);
		~ChunkMesh();

		void		draw();
		void		updateMesh();
		const GLuint &	getVAO();
		void		markToDelete();
		const bool &	checkToDelete();
};
