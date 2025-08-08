# pragma once

/// Defines
# define GLM_ENABLE_EXPERIMENTAL

/// System includes
# include <cstdint>
# include <cstring>
# include <list>
# include <unordered_map>

/// Dependencies
# include "glm/gtx/hash.hpp"
# include "AChunk.hpp"

enum	EnvParams {
	DRY = 0,
	DRENCHED,
	COLD,
	TEMPERATE,
	HOT
};

enum	BiomeID {
	PLAIN = 0,
	DESERT,
	FOREST,
	SNOW_PLAIN,
	SNOW_FOREST,
	NONE
};

# define WORLDFEATURE_THRESHOLDS	(float[]){10000.0f, 30.0f, 0.2f, 2.0f}

typedef struct WorldFeature {
	glm::ivec3	_localPosition;
	uint8_t		_type;
	glm::ivec4 *	_data;
	bool		_origin;
} WorldFeature;

enum e_worldFeatures {
	WF_NONE = 0,
	WF_CACTUS,
	WF_TREE,
	WF_SNOW_TREE
};

typedef struct WorldNoises {
	float *	heightMap;
	float * heatMap;
	float *	humidityMap;
	float * featuresMap;
}	WorldNoises;

/// Global variables
extern std::list<std::pair<glm::ivec3, WorldFeature> >	g_pendingFeatures;

// Chunk layer interface.
class	AChunkLayer {
	public:
		AChunkLayer();
		virtual ~AChunkLayer() = 0;

		virtual uint8_t	& operator[](const size_t &i) = 0;

		virtual void	print() = 0;
};

// Chunk layer when every blocks in the layer are the same.
// Only contains a single uint8_t to represent the block id of the whole layer.
class	SingleBlockChunkLayer : public AChunkLayer {
	private:
		uint8_t	_id;

	public:
		SingleBlockChunkLayer(const uint8_t &id);
		~SingleBlockChunkLayer();

		// Allways return the block id of the layered no matter the value i.
		// This avoid compatibility issues with other type of layers
		uint8_t	& operator[](const size_t &i);

		void	print();
};

// Default chunk layer.
// Stores an array of bytes that are indices to the chunk block palette.
class	ChunkLayer : public AChunkLayer {
	private:
		uint8_t	*_data;

	public:
		ChunkLayer(const uint8_t &id);
		~ChunkLayer();
	
		uint8_t	& operator[](const size_t &i);

		void	print();
};

// Default type of chunk.
// Contain a abstract type that stores a layer of block.
class	LayeredChunk : public AChunk {
	private:
		AChunkLayer **	_layer;

		float *		_computeHeatMap(const glm::ivec3 &pos);
		float *		_computeHeightMap(const glm::ivec3 &pos);
		float *		_computeHumidityMap(const glm::ivec3 &pos);
		float *		_computeFeatureMap(const glm::ivec3 &pos);
		float **	_computeCaveNoise(const glm::ivec3 &pos, float *heightMap);

		uint8_t	_getBiomeID(const int &idx, const float *heatFactors, const float *wetFactors);
		uint8_t	_getBlockFromBiome(const int &surface, const int &y, const uint8_t &biomeID);
		WorldFeature	_getFeatureFromBiome(const uint8_t &biomeID, const glm::ivec3 pos);
		void	_handleWorldFeatureOverflow(std::pair<glm::ivec3, WorldFeature> wf, glm::ivec3 newDir, const bool reset);
		
		ChunkLayer *		_blockToLayer(AChunkLayer *layer);
		SingleBlockChunkLayer *	_layerToBlock(AChunkLayer *layer);
	public:
		LayeredChunk(const uint8_t &id);
		~LayeredChunk();

		void	generate(const glm::ivec3 &pos);

		AChunkLayer *	& operator[](const size_t &i);

		// Debugging method. Will not be used in the final release.
		void	print();
		void	setBlock(const glm::ivec3 &pos, const uint8_t &blockID);
};

class	SingleBlockChunk : public AChunk {
	private:
		// This member will only be intenciated has a "SingleBlockChunkLayer".
		// An abstract type is used for compatibility with other chunk types.
		AChunkLayer	*_id;
	
	public:
		SingleBlockChunk(const uint8_t &id);
		~SingleBlockChunk();

		AChunkLayer *	& operator[](const size_t &i);
		
		void	print();
		void	generate(const glm::ivec3 &pos);
		void	setBlock(const glm::ivec3 &pos, const uint8_t &blockID);
};
