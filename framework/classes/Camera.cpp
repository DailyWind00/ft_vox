#include "Camera.hpp"
#include <glm/glm.hpp>

/// Constructors & Destructors
Camera::Camera(const CameraInfo &cameraInfo, const ProjectionInfo &projectionInfo, const ProjectionType &type) {
	_type = type;
	_cameraInfo = cameraInfo;
	_projectionInfo = projectionInfo;
	_updateViewMatrix();
	_updateProjectionMatrix();

	if (VERBOSE)
		std::cout << "Camera created" << std::endl;
}

Camera::Camera(const Camera &camera) {
	_type = camera._type;
	_cameraInfo = camera._cameraInfo;
	_projectionInfo = camera._projectionInfo;
	_updateViewMatrix();
	_updateProjectionMatrix();
}

Camera &Camera::operator=(const Camera &camera) {
	_type = camera._type;
	_cameraInfo = camera._cameraInfo;
	_projectionInfo = camera._projectionInfo;
	_updateViewMatrix();
	_updateProjectionMatrix();
	return *this;
}

Camera::~Camera() {
	if (VERBOSE)
		std::cout << "Camera destroyed" << std::endl;
}
/// ---



/// Private functions

// Update the view matrix
void	Camera::_updateViewMatrix() {
	_viewMatrix = glm::lookAt(_cameraInfo.position, _cameraInfo.lookAt, _cameraInfo.up);
	_cameraInfo.right = glm::normalize(glm::cross(glm::normalize(_cameraInfo.lookAt - _cameraInfo.position), _cameraInfo.up));
}

// Update the projection matrix
void	Camera::_updateProjectionMatrix() {
	switch (_type) {
		case ProjectionType::PERSPECTIVE:
			_projectionMatrix = glm::perspective(
				glm::radians(_projectionInfo.fov),
				_projectionInfo.resolution.x / _projectionInfo.resolution.y,
				_projectionInfo.near,
				_projectionInfo.far
			);
			break;
		case ProjectionType::ORTHOGRAPHIC:
			_projectionMatrix = glm::ortho(
				_projectionInfo.resolutionOffset.x,
				_projectionInfo.resolution.x,
				_projectionInfo.resolutionOffset.y,
				_projectionInfo.resolution.y,
				_projectionInfo.near, _projectionInfo.far
			);
			break;
	}
}
/// ---



/// Getters

// Return the camera information
const CameraInfo	&Camera::getCameraInfo() const {
	return _cameraInfo;
}

// Return the projection information
const ProjectionInfo	&Camera::getProjectionInfo() const {
	return _projectionInfo;
}

// Return the view matrix
glm::mat4	Camera::getViewMatrix() {
	return _viewMatrix;
}

// Return the projection matrix
glm::mat4	Camera::getProjectionMatrix() {
	return _projectionMatrix;
}

// Return the projection matrix * view matrix
Camera::operator glm::mat4() {
	return _projectionMatrix * _viewMatrix;
}
/// ---



/// Setters

// Set the camera information
void	Camera::setCameraInfo(const CameraInfo &cameraInfo) {
	_cameraInfo = cameraInfo;
	_updateViewMatrix();
}

// Set the projection type
void	Camera::setProjectionType(const ProjectionType &type) {
	_type = type;
	_updateProjectionMatrix();
}

// Set the camera position
void	Camera::setPosition(const glm::vec3 &position) {
	_cameraInfo.position = position;
	_updateViewMatrix();
}

// Set the camera lookAt
void	Camera::setLookAt(const glm::vec3 &lookAt) {
	_cameraInfo.lookAt = lookAt;
	_updateViewMatrix();
}

// Set the projection fov
void	Camera::setFOV(const float &fov) {
	_projectionInfo.fov = fov;
	_updateProjectionMatrix();
}

// Add to the camera position
void	Camera::addToPosition(const glm::vec3 &position) {
	_cameraInfo.position += position;
	_updateViewMatrix();
}

// Add to the camera lookAt
void	Camera::addToLookAt(const glm::vec3 &lookAt) {
	_cameraInfo.lookAt += lookAt;
	_updateViewMatrix();
}
/// ---
