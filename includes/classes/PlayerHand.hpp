# pragma once

# include <BufferGL.hpp>

class PlayerHand {
	private:
		// Render data
		GLuint		_VAO = 0;
		BufferGL *	_VBO = nullptr;
		
		// Block selection data
		std::vector<uint8_t>	_palette;
		int			_index;

	public:
		PlayerHand(const std::vector<uint8_t> &palette);
		~PlayerHand();

		void	draw();
		void	addToIndex(const int &inc);
		
		// Getters
		const uint8_t &	getID() const;
};
