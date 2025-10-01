#pragma once

/// Defines
# define GRAVITY_STRENGTH 0.0005f
# define GRAVITY_MAX     0.25f
# define GRAVITY_MIN    -1.0f

/// Dependencies
# include "BoundingBox.hpp"
# include "Camera.hpp"

using namespace glm;

class Player {
	private:
		Camera	&camera;
		AABB	&cameraBox;
		bool 	hasNoclip = false;
		bool 	hasFlashlight = false;

		// Gravity variables
		float gravity = 0.0f;
		bool canJump = true;
		bool isFalling = false;

	public:
		Player(const vec3 &startPos, Camera &cam, AABB &box);
		~Player() = default;

		/// Getters

		const vec3	&getPosition() const;
		Camera		&getCamera();
		AABB		&getCameraBox();
		bool		HaveNoclip() const;
		bool		HaveFlashlight() const;
		float		getGravity() const;
		bool		CanJump() const;
		bool		IsFalling() const;

		/// Setters

		void	setPosition(const vec3 &pos);
		void	translate(const vec3 &delta);
		void	setCamera(const Camera &cam);
		void	setCameraBox(const AABB &box);
		void	setNoclip(const bool &value);
		void	setFlashlight(const bool &value);
		void	setGravity(const float &value);
		void	setCanJump(const bool &value);
		void	setIsFalling(const bool &value);
};