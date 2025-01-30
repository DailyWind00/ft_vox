#pragma once

/// Defines
# define COLOR_HEADER_CXX
# define BASE_MAX_CHUNKS 1
# define CHUNK_SIZE 32
# define DATA_TYPE uint64_t

/// System includes
# include <iostream>
# include <vector>
# include <type_traits>

/// Dependencies
# include <glad/glad.h>
# include <glfw/glfw3.h>
# include "Noise.hpp"
# include "color.h"
# include "chunk.h"

/// Global variables
extern bool VERBOSE;

typedef std::vector<AChunk *> VChunks;

// Data structure for a Shader Storage Buffer Object
typedef struct SSBOData {
	glm::ivec4	worldPos;
} SSBOData;
typedef std::vector<SSBOData> VSSBOs;

// Data structure for a draw command (Indirect Buffer)
typedef struct {
    GLuint verticeCount;
    GLuint instanceCount;
    GLuint offset;
    GLuint baseInstance;
} DrawArraysIndirectCommand;
typedef std::vector<DrawArraysIndirectCommand> VDrawCommands;

// Core class for the voxel system
// Create chunks and their meshes & manage their rendering
class	VoxelSystem {
	// This class use persistent mapped buffers (VBOs) with glMultiDrawArraysIndirect
	// This allow:
	//   - Load/update/delete chunks efficiently
	//   - Batch all chunks in a single draw call
	private:
		GLuint			VAO;
		VChunks			chunks;
		
		// Vertex Buffer Object
		GLuint 			VBO;
		void		   *VBOdata = nullptr; // Persistent mapped VBO
		size_t			VBOcapacity = 0;
		size_t			currentVertexOffset = 0;

		// Indirect Buffer
		GLuint			IB;
		VDrawCommands	commands; // Stores the draw commands for each chunk
		size_t			IBcapacity = 0;

		// Shader Storage Buffer Object
		GLuint			SSBO;
		VSSBOs			chunksInfos;
		size_t			SSBOcapacity = 0;

		/// Private functions

		bool						isVoxelVisible(const size_t &x, const size_t &y, const size_t &z, AChunk *data);
		DrawArraysIndirectCommand 	genMesh(AChunk *chunk);
		void						createChunk(const glm::ivec3 &worldPos);

		// TODO :
		void						reallocateVBO(size_t newSize);
		// void						updateChunk(const glm::ivec3 &worldPos, const glm::mat4 &view); // Update the chunk mesh && load/unload chunks
		// void						deleteChunk(const glm::ivec3 &worldPos);

	public:
		VoxelSystem(); // Random seed
		VoxelSystem(const uint64_t &seed); // Custom seed
		~VoxelSystem();

		/// Public functions

		void	draw() const;
};
