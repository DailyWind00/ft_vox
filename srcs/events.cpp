#include "config.hpp"

// Handle the camera movements/interactions
static void	cameraMovement(Window &window, Camera &camera) {
	CameraInfo	cameraInfo = camera.getCameraInfo();

	/// Movements
	vec3	cameraFront = cameraInfo.lookAt - cameraInfo.position;
	vec3	cameraRight = normalize(cross(cameraFront, cameraInfo.up));
	cameraFront.y = 0; cameraRight.y = 0; // Remove the Y axis

	const float camSpeed = (CAMERA_SPEED + (CAMERA_SPRINT_BOOST * (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS))) * window.getFrameTime();

	vec3 move = vec3(0);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) move += cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) move -= cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) move -= cameraRight;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) move += cameraRight;
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) move += cameraInfo.up;
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) move -= cameraInfo.up;

	if (length(move) > 1.0f)
		move = normalize(move);

	cameraInfo.position += move * camSpeed;
	/// ---

	/// Mouse
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	static vec2	angles = vec2(0, 0);
	angles.x += (mouseX - ((float)WINDOW_WIDTH  / 2)) * CAMERA_SENSITIVITY * window.getFrameTime();
	angles.y -= (mouseY - ((float)WINDOW_HEIGHT / 2)) * CAMERA_SENSITIVITY * window.getFrameTime();
	clamp(angles.y, -89.0f, 89.0f);

	vec3		cameraDir = vec3{
		cos(radians(angles.x)) * cos(radians(angles.y)),
		sin(radians(angles.y)),
		sin(radians(angles.x)) * cos(radians(angles.y))
	};

	cameraInfo.lookAt = cameraInfo.position + cameraDir;

	glfwSetCursorPos(window, (float)WINDOW_WIDTH / 2, (float)WINDOW_HEIGHT / 2);
	/// ---
	
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

	float		dayDuration = 200;
	float		angle = (time / dayDuration) * M_PI;
	vec3	sunPos = normalize(vec3(cosf(angle), sinf(angle), 0.0f));

	mat4 skyboxView = camera.getProjectionMatrix() * mat4(mat3(camera.getViewMatrix())); // Get rid of the translation part
	shaders.setUniform((*shaders[0])->getID(), "time", time);
	shaders.setUniform((*shaders[0])->getID(), "camera", skyboxView);
	shaders.setUniform((*shaders[0])->getID(), "sunPos", sunPos);

	// Geometrie Pass Shader parameters
	shaders.setUniform((*shaders[1])->getID(), "transform", camera);
	shaders.setUniform((*shaders[1])->getID(), "time", time);

	// Lighting Pass Shader Parameters
	shaders.setUniform((*shaders[2])->getID(), "time", time);
	shaders.setUniform((*shaders[2])->getID(), "camera", skyboxView);
	shaders.setUniform((*shaders[2])->getID(), "sunPos", sunPos);
	shaders.setUniform((*shaders[2])->getID(), "gPosition", 0);
	shaders.setUniform((*shaders[2])->getID(), "gNormal", 1);
	shaders.setUniform((*shaders[2])->getID(), "gColor", 2);
}
