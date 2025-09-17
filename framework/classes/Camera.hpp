#pragma once

/// System includes
# include <iostream>

/// Dependencies
# include "glm/glm.hpp"
# include <glm/gtc/matrix_transform.hpp>

/// Global variables
extern bool	VERBOSE;

typedef enum class ProjectionType {
	PERSPECTIVE,
	ORTHOGRAPHIC
} ProjectionType;

// Struct to store the camera information (view matrix)
typedef struct CameraInfo {
	glm::vec3	position = {0, 0, 0};
	glm::vec3	lookAt   = {0, 1, 0};
	glm::vec3	up       = {0, 0, 1};
	glm::vec3	right    = {1, 0, 0};
} CameraInfo;

// Struct to store the projection information (projection matrix)
typedef struct ProjectionInfo {
	float	fov          = 45.0f;
	float	aspectRatio  = 16.0f / 9.0f;
	float	near         = 0.1f;
	float	far          = 100.0f;
} ProjectionInfo;

// Class for camera management, handle the view and projection matrices and the camera position
class Camera {
	private:
		CameraInfo		_cameraInfo;
		ProjectionInfo	_projectionInfo;

		glm::mat4		_viewMatrix;
		glm::mat4		_projectionMatrix;

		ProjectionType	_type = ProjectionType::PERSPECTIVE;

		/// Private functions

		void	_updateViewMatrix();
		void	_updateProjectionMatrix();

	public:
		Camera(const CameraInfo &cameraInfo, const ProjectionInfo &projectionInfo);
		Camera(const Camera &camera);
		Camera &operator=(const Camera &camera);
		~Camera();

		/// Getters
		
		const CameraInfo		&getCameraInfo() const;
		const ProjectionInfo	&getProjectionInfo() const;

		glm::mat4	getViewMatrix();
		glm::mat4	getProjectionMatrix();
		operator	glm::mat4();

		/// Setters

		void	setCameraInfo(const CameraInfo &cameraInfo);
		void	setProjectionType(const ProjectionType &type);

		void	setPosition(const glm::vec3 &position);
		void	setLookAt(const glm::vec3 &lookAt);

		void	addToPosition(const glm::vec3 &position);
		void	addToLookAt(const glm::vec3 &lookAt);

		void	setFOV(const float &fov);
};