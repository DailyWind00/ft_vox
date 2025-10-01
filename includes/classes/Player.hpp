#pragma once

/// System includes

/// Dependencies
# include "BoundingBox.hpp"
# include "Camera.hpp"

using namespace glm;

class Player {
	private:
		Camera camera;
		AABB cameraBox;
		bool hasNoclip = false;
		bool hasFlashlight = false;

	public:
		Player(const vec3 &startPos, const Camera &cam, const AABB &box);
		~Player() = default;

		/// Getters

		const vec3	&getPosition() const;
		Camera		&getCamera();
		AABB		&getCameraBox();
		bool		HaveNoclip() const;
		bool		HaveFlashlight() const;

		/// Setters

		void	setPosition(const vec3 &pos);
		void	translate(const vec3 &delta);
		void	setCamera(const Camera &cam);
		void	setCameraBox(const AABB &box);
		void	setNoclip(const bool &value);
		void	setFlashlight(const bool &value);
};