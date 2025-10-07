# include <PlayerHand.hpp>

PlayerHand::PlayerHand(const std::vector<uint8_t> &palette) : _palette(palette), _index(0) {
	// Generating hand's vertex array
	glGenVertexArrays(1, &_VAO);
	glBindVertexArray(_VAO);

	float	data[] = {
		// X face
		-0.175f, -0.175f, 0.175f, 0.0f, 0.0f, 0,
		-0.175f,  0.175f, 0.175f, 0.0f, 1.0f, 0,
		 0.175f, -0.175f, 0.175f, 1.0f, 0.0f, 0,
		 0.175f,  0.175f, 0.175f, 1.0f, 1.0f, 0,
		 0.175f, -0.175f, 0.175f, 1.0f, 0.0f, 0,
		-0.175f,  0.175f, 0.175f, 0.0f, 1.0f, 0,

		// Y face
		-0.175f, 0.175f, -0.175f, 0.0f, 0.0f, 1,
		 0.175f, 0.175f, -0.175f, 0.0f, 1.0f, 1,
		-0.175f, 0.175f,  0.175f, 1.0f, 0.0f, 1,
		 0.175f, 0.175f,  0.175f, 1.0f, 1.0f, 1,
		-0.175f, 0.175f,  0.175f, 1.0f, 0.0f, 1,
		 0.175f, 0.175f, -0.175f, 0.0f, 1.0f, 1,

		 // Z face
		 -0.175f, -0.175f, -0.175f, 0.0f, 0.0f, 2,
		 -0.175f, -0.175f,  0.175f, 1.0f, 0.0f, 2,
		 -0.175f,  0.175f, -0.175f, 0.0f, 1.0f, 2,
		 -0.175f,  0.175f,  0.175f, 1.0f, 1.0f, 2,
		 -0.175f,  0.175f, -0.175f, 0.0f, 1.0f, 2,
		 -0.175f, -0.175f,  0.175f, 1.0f, 0.0f, 2
	};

	// Generating hand's vertex buffer
	_VBO = new BufferGL(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(data), data);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, nullptr);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 1, GL_INT, GL_FALSE, sizeof(float) * 6, (void *)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);
	
	glBindVertexArray(0);
}

PlayerHand::~PlayerHand() {

}

void	PlayerHand::addToIndex(const int &inc) {
	if (_index + inc < 0)
		_index = _palette.size() - 1;
	else if (_index + inc > (int)_palette.size() - 1)
		_index = 0;
	else
		_index += inc;
}

void	PlayerHand::draw() {
	glBindVertexArray(_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 18);
	glBindVertexArray(0);
}

const uint8_t &	PlayerHand::getID() const {
	return _palette[_index];
}
