#include "config.hpp"

// Handle the camera movements/interactions
static glm::mat4 cameraHandler(Window &window) {
	static glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, -10.0f);
	static glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
	static glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	static glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));
	
	cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));

	float	camCurrentSpeed = CAMERA_SPEED + (CAMERA_SPRINT_BOOST * (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS));

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		cameraPos.x += cameraFront.x * camCurrentSpeed;
		cameraPos.z += cameraFront.z * camCurrentSpeed;
	}
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		cameraPos.x -= cameraFront.x * camCurrentSpeed;
		cameraPos.z -= cameraFront.z * camCurrentSpeed;
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		cameraPos.x += cameraRight.x * camCurrentSpeed;
		cameraPos.z += cameraRight.z * camCurrentSpeed;
	}
	else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		cameraPos.x -= cameraRight.x * camCurrentSpeed;
		cameraPos.z -= cameraRight.z * camCurrentSpeed;
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		cameraPos += cameraUp * glm::vec3{camCurrentSpeed};
	else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		cameraPos -= cameraUp * glm::vec3{camCurrentSpeed};

	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	static glm::vec2	angles = glm::vec2(0, 0);

	angles.x += (mouseX - ((float)WINDOW_WIDTH / 2)) * CAMERA_SENSITIVITY;
	angles.y -= (mouseY - ((float)WINDOW_HEIGHT / 2)) * CAMERA_SENSITIVITY;

	if (angles.y > 89)
		angles.y = 89;
	if (angles.y < -89)
		angles.y = -89;
	
	glm::vec3		cameraDir = glm::vec3{
		cos(glm::radians(angles.x)) * cos(glm::radians(angles.y)),
		sin(glm::radians(angles.y)),
		sin(glm::radians(angles.x)) * cos(glm::radians(angles.y))
	};
	cameraFront = glm::normalize(cameraDir);
	
	glfwSetCursorPos(window, (float)WINDOW_WIDTH / 2, (float)WINDOW_HEIGHT / 2);

	return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

// Handle all keyboard & other events
void	handleEvents(Window &window, Shader &shader)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = cameraHandler(window);
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

	shader.setUniform("transform", projection * view * model);
}
