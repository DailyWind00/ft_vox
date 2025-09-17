# pragma once

/// Defines
# define DATA_TYPE	uint64_t

/// System includes
# include <vector>

/// Dependencies
# include "BufferGL.hpp"

class	ChunkMesh {
	private:
		GLuint		_VAO = 0;
		BufferGL *	_VBO = nullptr;

		std::vector<DATA_TYPE>	_data;
		size_t		_dataSize = 0;
	
	public:
		ChunkMesh(const std::vector<DATA_TYPE> &data);
		~ChunkMesh();

		void		draw();
		void		updateMesh();
		const GLuint &	getVAO() const;
};
