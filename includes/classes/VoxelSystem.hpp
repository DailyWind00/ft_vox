#pragma once

/// Defines
# define COLOR_HEADER_CXX

/// System includes
# include <iostream>
# include <vector>

/// Dependencies
# include <glad/glad.h>
# include <glfw/glfw3.h>
# include "Noise.hpp"
# include "color.h"
# include "chunk.h"

/// Global variables
extern bool VERBOSE;

// Data for a chunk
typedef struct chunkData {
	AChunk	   *chunk;
	glm::ivec3	worldPos;
} chunkData;

typedef struct {
    GLuint verticeCount;
    GLuint instanceCount;
    GLuint offset;
    GLuint baseInstance;
} DrawArraysIndirectCommand;
typedef std::vector<DrawArraysIndirectCommand> VDrawCommands;

// Core class for the voxel system
// Create chunks and their meshes & manage their rendering
// dataType: The type of the vertices (float, double, ...)
template <typename dataType = float>
class	VoxelSystem {
	// This class use persistent mapped buffers (VBOs) with glMultiDrawArraysIndirect
	// This allow:
	//   - Load/update/delete chunks efficiently
	//   - Batch all chunks in a single draw call
	private:
		std::vector<chunkData>	chunks;

		// OpenGL data
		GLuint			VAO;
		GLuint 			VBO;
		GLuint			IB; // Indirect buffer
		void		   *VBOdata = nullptr; // Persistent mapped VBO
		size_t			currentVertexOffset = 0;
		VDrawCommands	commands; // Stores the draw commands for each chunk

	public:
		VoxelSystem(); // Random seed
		VoxelSystem(const uint64_t &seed); // Custom seed
		~VoxelSystem();

		/// Public functions

		void	createChunk(const glm::ivec3 &worldPos); // May be a private function or like a dynamic loading
		void	draw() const;
};
