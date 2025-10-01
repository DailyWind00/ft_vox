# include "BoundingBox.hpp"

# pragma region AABB class

AABB::AABB(const glm::vec3& min, const glm::vec3& max) : min(min), max(max) {}

/// Public functions

// Draw the AABB (generally used for debugging)
void AABB::draw() const {
	const std::array<glm::vec3, 8> corners = {
		glm::vec3(min.x, min.y, min.z), // 0
		glm::vec3(max.x, min.y, min.z), // 1
		glm::vec3(max.x, max.y, min.z), // 2
		glm::vec3(min.x, max.y, min.z), // 3
		glm::vec3(min.x, min.y, max.z), // 4
		glm::vec3(max.x, min.y, max.z), // 5
		glm::vec3(max.x, max.y, max.z), // 6
		glm::vec3(min.x, max.y, max.z)  // 7
	};
    const unsigned int indices[24] = {
        0,1, 1,2, 2,3, 3,0, // Back face
        4,5, 5,6, 6,7, 7,4, // Front face
        0,4, 1,5, 2,6, 3,7  // Connectors
    };

	// Generate temporary VAO/VBO (fine for debug)
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, corners.size() * sizeof(glm::vec3), corners.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
    glEnableVertexAttribArray(0);

	// Draw the box
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);

	// Cleanup
	glBindVertexArray(0);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}

// Check if this AABB intersects with another bounding box
bool AABB::intersects(const BoundingBox& other) const {
	if (other.getType() == Type::AABB) {
		const AABB& o = static_cast<const AABB&>(other);
		return (min.x <= o.max.x && max.x >= o.min.x) &&
			   (min.y <= o.max.y && max.y >= o.min.y) &&
			   (min.z <= o.max.z && max.z >= o.min.z);
	}
	else if (VERBOSE)
		std::cerr << BRed << "AABB::intersects: Intersection with other bounding box types not implemented yet." << ResetColor << std::endl;
	return false;
}

// Check if the given point is in the AABB
bool AABB::contains(const glm::vec3& point) const {
	return (point.x >= min.x && point.x <= max.x) &&
		   (point.y >= min.y && point.y <= max.y) &&
		   (point.z >= min.z && point.z <= max.z);
}

/// Getters

// Return the type of the bounding box
BoundingBox::Type AABB::getType() const {
	return Type::AABB;
}

/// Setters

// Set the min and max points of the AABB
void AABB::set(const glm::vec3& min, const glm::vec3& max) {
	this->min = min;
	this->max = max;
}

// Set the position of the center of the AABB to the given position
void AABB::setPosition(const glm::vec3& position) {
	glm::vec3 size = max - min;
	this->min = position - size * 0.5f;
	this->max = position + size * 0.5f;
}

// Translate the AABB by the given offset
void AABB::translate(const glm::vec3& offset) {
	this->min += offset;
	this->max += offset;
}

# pragma endregion // AABB class