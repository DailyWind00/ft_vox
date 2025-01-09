/// class independant system includes
# include <iostream>

# include "Chunk.hpp"

Chunk::Chunk() : _node(new ChunkNode), _eMax(5) {}
Chunk::Chunk(const uint16_t &filler) : _node(new ChunkNode), _eMax(5)
{
	this->_node->_blockInfos |= filler;
	//-for (int i = 11; i <= 13; i++)
		//-this->_node->_blockInfos |= (0 << i);
	this->_node->_blockInfos |= (1 << 14);
	//-for (int i = 15; i <= 16; i++)
		//-this->_node->_blockInfos |= (0 << i);
	
	std::cout << this->_node->_blockInfos << std::endl;
}

Chunk::~Chunk() {}

uint16_t	Chunk::getBlockAt(const glm::uvec3 &pos)
{
	(void)pos;
	return (0);
}

void	setBlockAt(const glm::uvec3 &pos)
{
	(void)pos;
}

/// Chunk Node operator overload
ChunkNode	* ChunkNode::operator[](const uint8_t &idx)
{
	if (idx < 7)
		return (this->_n[idx]);
	return (NULL);
}
