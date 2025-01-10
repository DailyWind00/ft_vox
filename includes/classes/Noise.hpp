# pragma once

/// Defines

/// System includes
# include <cstdlib>

/// Dependencies
# include <glm/glm.hpp>

/// Global variables

class	Noise {
	private:
		Noise() {}
		~Noise() {}

		static float		_perlin2DDot(const glm::ivec2 &v1, const glm::vec2 &v2, const uint32_t &randFactor);
		static float		_perlin2DCubInterpol(const glm::vec2 &v, const float &weight);
		static glm::vec2	_perlin2DRandomGradiant(const glm::ivec2 &v, const uint32_t &randFactor);

	public:
		static void	setSeed(const uint64_t &seed);
		static float	perlin2D(const glm::vec2 &v, const uint32_t &randFactor);
		static float	perlin3D();
};
