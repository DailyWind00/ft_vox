# include <CloudSystem.hpp>
# include <Noise.hpp>
# include <vector>

CloudSystem::CloudSystem(const size_t &sampleSize) {
	_cloudSampleSize = sampleSize;

	// Create the noise sample data
	std::vector<float>	data;
	for (int i = -(_cloudSampleSize / 2.0f); i < _cloudSampleSize / 2.0f; i++) {
		for (int k = -(_cloudSampleSize / 2.0f); k < _cloudSampleSize / 2.0f; k++) {
			glm::vec2	samplePoint = {abs(i), abs(k)};
			float	noise = Noise::perlin2D(samplePoint / 32.0f) * 2048.0f;

			noise += Noise::perlin2D(samplePoint / 16.0f) * 1024.0f;
			noise += Noise::perlin2D(samplePoint / 4.0f) * 1024.0f;
			data.push_back(noise);
		}
	}

	// Create the 3D texture to store the sample data
	glGenTextures(1, &_cloudNoiseSample3D);
	glBindTexture(GL_TEXTURE_3D, _cloudNoiseSample3D);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, _cloudSampleSize, 1.0f, _cloudSampleSize, 0, GL_RED, GL_FLOAT, data.data());
}

CloudSystem::~CloudSystem() {
	glDeleteTextures(1, &_cloudNoiseSample3D);
}

const GLuint &	CloudSystem::getCloudNoiseSample() const {
	return _cloudNoiseSample3D;
}
