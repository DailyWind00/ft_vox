# include "Player.hpp"

#pragma region Constructor / Destructor

Player::Player(const vec3 &startPos, Camera &cam, AABB &box) : camera(cam), cameraBox(box) {
	camera.setPosition(startPos);
	cameraBox.setPosition(startPos);
}

# pragma endregion

# pragma region Getters

/// @brief Get the player's position
/// @return The player's position as a vec3
const vec3 & Player::getPosition() const {
	return camera.getCameraInfo().position;
}

/// @brief Get the player's camera
/// @return The player's camera as a Camera
Camera & Player::getCamera() {
	return camera;
}

/// @brief Get the player's camera bounding box
/// @return The player's camera bounding box as an AABB
AABB & Player::getCameraBox() {
	return cameraBox;
}

/// @brief Check if the player has noclip mode enabled
/// @return True if noclip mode is enabled, false otherwise
bool 	Player::HaveNoclip() const {
	return hasNoclip;
}

/// @brief Check if the player has the flashlight enabled
/// @return True if the flashlight is enabled, false otherwise
bool 	Player::HaveFlashlight() const {
	return hasFlashlight;
}

/// @brief Get the player's gravity
/// @return The player's gravity as a float
float Player::getGravity() const {
	return gravity;
}

/// @brief Check if the player can jump
/// @return True if the player can jump, false otherwise
bool Player::CanJump() const {
	return canJump;
}

/// @brief Check if the player is falling
/// @return True if the player is falling, false otherwise
bool Player::IsFalling() const {
	return isFalling;
}

# pragma endregion


# pragma region Setters

/// @brief Set the player's position and update the camera and bounding box positions accordingly
/// @param pos New position of the player
void	Player::setPosition(const vec3 &pos) {
	camera.setPosition(pos);
	cameraBox.setPosition(pos);
}

/// @brief Translate the player by a given offset
/// @param delta Offset by which to translate the player
void	Player::translate(const vec3 &delta) {
	camera.addToPosition(delta);
	cameraBox.translate(delta);
}

/// @brief Set the player's camera
/// @param cam New camera to be set for the player
void	Player::setCamera(const Camera &cam) {
	camera = cam;
}

/// @brief Set the player's camera bounding box
/// @param box New camera bounding box to be set for the player
void	Player::setCameraBox(const AABB &box) {
	cameraBox = box;
}

/// @brief Enable or disable noclip mode for the player
/// @param value Boolean value to set noclip mode (true to enable, false to disable)
void	Player::setNoclip(const bool &value) {
	hasNoclip = value;
}

/// @brief Enable or disable the flashlight for the player
/// @param value Boolean value to set flashlight state (true to enable, false to disable)
void	Player::setFlashlight(const bool &value) {
	hasFlashlight = value;
}

/// @brief Set the player's gravity
/// @param value New gravity value to be set for the player
void	Player::setGravity(const float &value) {
	gravity = value;
}

/// @brief Set whether the player can jump
/// @param value Boolean value to set jump ability (true to allow jumping, false to disable)
void	Player::setCanJump(const bool &value) {
	canJump = value;
}

/// @brief Set whether the player is falling
/// @param value Boolean value to set falling state (true to enable falling, false to disable)
void	Player::setIsFalling(const bool &value) {
	isFalling = value;
}

# pragma endregion