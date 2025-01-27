#include "config.hpp"

// Handle the camera movements/interactions
static glm::mat4 cameraHandler(Window &window) {
	static glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, -10.0f);
	static glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);

	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	if (mouseX < WINDOW_WIDTH / 2)
		cameraPos.x += CAMERA_SPEED;
	else if (mouseX > WINDOW_WIDTH / 2)
		cameraPos.x -= CAMERA_SPEED;

	if (mouseY < WINDOW_HEIGHT / 2)
		cameraPos.y += CAMERA_SPEED;
	else if (mouseY > WINDOW_HEIGHT / 2)
		cameraPos.y -= CAMERA_SPEED;

	glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

	return glm::lookAt(cameraPos, cameraPos + cameraFront, glm::vec3(0.0f, 1.0f, 0.0f));
}

// Handle all keyboard & other events
void	handleEvents(Window &window, Shader &shader) {
	glfwPollEvents();

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = cameraHandler(window);
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

	shader.setUniform("transform", projection * view * model);
}