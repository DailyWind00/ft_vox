# include "BoundingBox.hpp"

# pragma region AABB class

AABB::AABB(const glm::vec3& min, const glm::vec3& max) : min(min), max(max) {}

/// Public functions

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

// Translate the AABB by the given offset
void AABB::translate(const glm::vec3& offset) {
	this->min += offset;
	this->max += offset;
}

# pragma endregion // AABB class