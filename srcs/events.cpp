#include "config.hpp"

// Handle the camera movements/interactions
static void	cameraMovement(Window &window, Camera &camera) {
	CameraInfo	cameraInfo = camera.getCameraInfo();
	glm::vec3	cameraFront = cameraInfo.lookAt - cameraInfo.position;
	glm::vec3	cameraRight = glm::normalize(glm::cross(cameraFront, cameraInfo.up));

	float camSpeed = (CAMERA_SPEED + (CAMERA_SPRINT_BOOST * (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS))) * window.getFrameTime();

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraInfo.position += cameraFront * camSpeed;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraInfo.position -= cameraFront * camSpeed;

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraInfo.position -= cameraRight * camSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraInfo.position += cameraRight * camSpeed;

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		cameraInfo.position += cameraInfo.up * camSpeed;
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		cameraInfo.position -= cameraInfo.up * camSpeed;

	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	static glm::vec2	angles = glm::vec2(0, 0);

	angles.x += (mouseX - ((float)WINDOW_WIDTH / 2)) * CAMERA_SENSITIVITY * window.getFrameTime();
	angles.y -= (mouseY - ((float)WINDOW_HEIGHT / 2)) * CAMERA_SENSITIVITY * window.getFrameTime();

	if (angles.y > 89)
		angles.y = 89;
	if (angles.y < -89)
		angles.y = -89;

	glm::vec3		cameraDir = glm::vec3{
		cos(glm::radians(angles.x)) * cos(glm::radians(angles.y)),
		sin(glm::radians(angles.y)),
		sin(glm::radians(angles.x)) * cos(glm::radians(angles.y))
	};

	cameraInfo.lookAt = cameraInfo.position + glm::normalize(cameraDir);

	glfwSetCursorPos(window, (float)WINDOW_WIDTH / 2, (float)WINDOW_HEIGHT / 2);

	camera.setCameraInfo(cameraInfo);
}

// Handle all keyboard & other events
void	handleEvents(GameData &gameData) {
	Window			&window  = gameData.window;
	ShaderHandler	&shaders = gameData.shaders;
	Camera			&camera  = gameData.camera;

	static float time = 0; time += 0.01;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	cameraMovement(window, camera);

	glm::mat4 skyboxView = camera.getProjectionMatrix() * glm::mat4(glm::mat3(camera.getViewMatrix())); // Get rid of the translation part
	shaders.setUniform((*shaders[0])->getID(), "time", time);
	shaders.setUniform((*shaders[0])->getID(), "camera", skyboxView);
	shaders.setUniform((*shaders[1])->getID(), "transform", camera);
}
