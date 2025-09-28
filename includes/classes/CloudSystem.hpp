# pragma once

# include <glad/glad.h>
# include <cstdlib>

class CloudSystem {
	private:
		GLuint	_cloudNoiseSample3D;
		size_t	_cloudSampleSize;

	public:
		CloudSystem(const size_t &sampleSize);
		~CloudSystem();

		const GLuint &	getCloudNoiseSample() const;
};
