
# include "Noise.hpp"

unsigned int	g_randomFactor = 0;

void	Noise::setSeed(const uint64_t &seed)
{
	srand(seed);
	g_randomFactor = 1 + rand();
}

float	Noise::perlin2D(const glm::vec2 &v)
{
	glm::ivec2	topLeft = {
		(int)v.x, (int)v.y
	};
	glm::ivec2	topRight = {
		(int)v.x + 1, (int)v.y
	};
	glm::ivec2	bottomLeft = {
		(int)v.x, (int)v.y + 1
	};
	glm::ivec2	bottomRight = {
		(int)v.x + 1, (int)v.y + 1
	};

	glm::vec2	wVector = {
		v.x - (float)topLeft.x,
		v.y - (float)topLeft.y
	};

	return (_perlin2DCubInterpol(glm::vec2{
			_perlin2DCubInterpol(glm::vec2{_perlin2DDot(topLeft, v),
						_perlin2DDot(topRight, v)}, wVector.x),
			_perlin2DCubInterpol(glm::vec2{_perlin2DDot(bottomLeft, v),
						_perlin2DDot(bottomRight, v)}, wVector.x)},
		wVector.y));
}

float	Noise::_perlin2DDot(const glm::ivec2 &v1, const glm::vec2 &v2)
{
	glm::vec2	gradiant = _perlin2DRandomGradiant(v1);

	return (glm::dot(glm::vec2{v2.x - (float)v1.x,
				v2.y - (float)v1.y}, gradiant));
}

float	Noise::_perlin2DCubInterpol(const glm::vec2 &v, const float &weight)
{
	return ((v.y - v.x) * (3.0f - weight * 2.0f) * weight * weight + v.x);
}

glm::vec2	Noise::_perlin2DRandomGradiant(const glm::ivec2 &v)
{
	const unsigned int	w = 8 * sizeof(unsigned);
	const unsigned int	s = w / 2; 
	unsigned int		a = v.x + g_randomFactor, b = v.y + g_randomFactor;
	
	a *= 3284157443;
 
	b ^= a << s | a >> (w - s);
	b *= 1911520717;
 
	a ^= b << s | b >> (w - s);
	a *= 2048419325;
	float	random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]
    
	return  glm::vec2{sin(random), cos(random)};
}
