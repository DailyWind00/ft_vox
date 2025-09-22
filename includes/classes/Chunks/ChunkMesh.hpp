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

		GLuint		_waterVAO = 0;
		BufferGL *	_waterVBO = nullptr;

		std::vector<DATA_TYPE>	_data;
		size_t		_dataSize = 0;

		std::vector<DATA_TYPE>	_waterData;
		size_t		_waterDataSize = 0;
	
	public:
		ChunkMesh(const std::vector<DATA_TYPE> &data, const std::vector<DATA_TYPE> &waterData);
		~ChunkMesh();

		void		draw();
		void		drawWater();
		void		updateMesh();
		const GLuint &	getVAO() const;
		const GLuint &	getWaterVAO() const;
};
