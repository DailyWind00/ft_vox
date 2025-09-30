#pragma once

/// Defines
# define COLOR_HEADER_CXX

/// System includes
# include <iostream>

/// Dependencies
# include <glm/glm.hpp>
# include "color.h"

/// Global variables
extern bool VERBOSE;

// Abstract base class for different types of bounding boxes
class BoundingBox {
	protected:
		enum class Type {
			SS,
			AABB,
			OBB,
			FDH,
			CH
		};

	public:
		virtual ~BoundingBox() = default;

		/// Public functions

		virtual bool intersects(const BoundingBox& other) const = 0;
		virtual bool contains(const glm::vec3& point) const = 0;

		/// Getters

    	virtual Type getType() const = 0;

		/// Setters

		virtual void set(const glm::vec3& min, const glm::vec3& max) = 0;
		virtual void translate(const glm::vec3& offset) = 0;
		
};

// Axis-Aligned Bounding Box (AABB) class
class AABB : public BoundingBox {
	private:
		glm::vec3 min;
		glm::vec3 max;

	public:
		AABB(const glm::vec3& min = glm::vec3(0.0f), const glm::vec3& max = glm::vec3(1.0f));
		~AABB() override = default;

		/// Public functions

		bool intersects(const BoundingBox& other) const override;
		inline bool contains(const glm::vec3& point) const override;

		/// Getters

		inline Type getType() const override;

		/// Setters

		void set(const glm::vec3& min, const glm::vec3& max) override;
		void translate(const glm::vec3& offset) override;
};