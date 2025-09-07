#include "config.hpp"

#pragma region Keys Pressed Once

// Check if a key is pressed once
// Handle multiple keys
// Returns true a single time if the key has been pressed, false otherwise
static inline bool keyPressedOnce(GLFWwindow *window, int key) {
	static unordered_map<int, bool> pressed;

	if (glfwGetKey(window, key) == GLFW_PRESS && !pressed[key]) {
		pressed[key] = true;
		return true;
	}
	else if (glfwGetKey(window, key) == GLFW_RELEASE)
		pressed[key] = false;

	return false;
}

// Check if a mouse button is pressed once
// Handle multiple buttons
// Returns true a single time if the button has been pressed, false otherwise
static inline bool MouseButtonPressedOnce(GLFWwindow *window, int button) {
	static unordered_map<int, bool> pressed;

	if (glfwGetMouseButton(window, button) == GLFW_PRESS && !pressed[button]) {
		pressed[button] = true;
		return true;
	}
	else if (glfwGetMouseButton(window, button) == GLFW_RELEASE)
		pressed[button] = false;

	return false;
}

#pragma endregion

// Handle the request for chunks/meshes creation_update/deletion
static void	chunkRequests(VoxelSystem &voxelSystem, const CameraInfo &cameraInfo)
{
	glm::ivec3		camChunkPos = {
		cameraInfo.position.x / CHUNK_SIZE,
		cameraInfo.position.y  / CHUNK_SIZE,
		cameraInfo.position.z / CHUNK_SIZE
	};
	static glm::ivec3	prevCamChunkPos = camChunkPos;

	// Wait for the current chunks to be generated before sending other ones
	if (voxelSystem.getChunkRequestCount() != 0)
		return ;

	vector<ChunkRequest>	chunks;
	static int		currentRequestDistanceX = glm::min(SPAWN_LOCATION_SIZE, HORIZONTAL_RENDER_DISTANCE) + 1;
	static int		currentRequestDistanceZ = glm::min(SPAWN_LOCATION_SIZE, HORIZONTAL_RENDER_DISTANCE) + 1;

	if (camChunkPos == prevCamChunkPos) {	// If the player is staying in the same chunk, the world load until the render distance is reached
		if (currentRequestDistanceX >= HORIZONTAL_RENDER_DISTANCE || currentRequestDistanceZ >= HORIZONTAL_RENDER_DISTANCE)
			return ;
		for (int i = VERTICAL_RENDER_DISTANCE; i >= -VERTICAL_RENDER_DISTANCE; i--) {
			for (int j = -currentRequestDistanceX; j <= currentRequestDistanceX; j++) {
				chunks.push_back({{-currentRequestDistanceX + camChunkPos.x, i + camChunkPos.y, j + camChunkPos.z}, ChunkAction::CREATE_UPDATE});
				chunks.push_back({{currentRequestDistanceX + camChunkPos.x, i + camChunkPos.y, j + camChunkPos.z}, ChunkAction::CREATE_UPDATE});
			}
			for (int j = -currentRequestDistanceZ + 1; j <= currentRequestDistanceZ - 1; j++) {
				chunks.push_back({{j + camChunkPos.x, i + camChunkPos.y, -currentRequestDistanceZ + camChunkPos.z}, ChunkAction::CREATE_UPDATE});
				chunks.push_back({{j + camChunkPos.x, i + camChunkPos.y, currentRequestDistanceZ + camChunkPos.z}, ChunkAction::CREATE_UPDATE});
			}
		}
		if (currentRequestDistanceX < HORIZONTAL_RENDER_DISTANCE)
			currentRequestDistanceX++;
		if (currentRequestDistanceZ < HORIZONTAL_RENDER_DISTANCE)
			currentRequestDistanceZ++;
	}
	else {
		currentRequestDistanceX = 1;
		currentRequestDistanceZ = 1;
		for (int i = VERTICAL_RENDER_DISTANCE; i >= -VERTICAL_RENDER_DISTANCE; i--) {
			for (int j = -HORIZONTAL_RENDER_DISTANCE; j <= HORIZONTAL_RENDER_DISTANCE; j++) {
				chunks.push_back({{-HORIZONTAL_RENDER_DISTANCE + camChunkPos.x, i + camChunkPos.y, j + camChunkPos.z}, ChunkAction::DELETE});
				chunks.push_back({{HORIZONTAL_RENDER_DISTANCE + camChunkPos.x, i + camChunkPos.y, j + camChunkPos.z}, ChunkAction::DELETE});
			}
			for (int j = -HORIZONTAL_RENDER_DISTANCE; j <= HORIZONTAL_RENDER_DISTANCE; j++) {
				chunks.push_back({{j + camChunkPos.x, i + camChunkPos.y, -HORIZONTAL_RENDER_DISTANCE + camChunkPos.z}, ChunkAction::DELETE});
				chunks.push_back({{j + camChunkPos.x, i + camChunkPos.y, HORIZONTAL_RENDER_DISTANCE + camChunkPos.z}, ChunkAction::DELETE});
			}
		}
		for (int i = -HORIZONTAL_RENDER_DISTANCE; i < HORIZONTAL_RENDER_DISTANCE; i++) {
			for (int j = -HORIZONTAL_RENDER_DISTANCE; j < HORIZONTAL_RENDER_DISTANCE; j++) {
				chunks.push_back({{j + camChunkPos.x, -VERTICAL_RENDER_DISTANCE + camChunkPos.y, i + camChunkPos.z}, ChunkAction::DELETE});
				chunks.push_back({{j + camChunkPos.x, VERTICAL_RENDER_DISTANCE + camChunkPos.y, i + camChunkPos.z}, ChunkAction::DELETE});
			}
		}
	}

	voxelSystem.requestChunk(chunks);
	prevCamChunkPos = camChunkPos;
}

// Handle the camera movements/interactions
static void	cameraMovement(Window &window, Camera &camera) {
	CameraInfo	cameraInfo = camera.getCameraInfo();

	# pragma region Camera controls
	vec3	cameraFront = cameraInfo.lookAt - cameraInfo.position;
	vec3	cameraRight = normalize(cross(cameraFront, cameraInfo.up));
	cameraFront.y = 0; cameraRight.y = 0; // Remove the Y axis
	cameraFront = normalize(cameraFront);
	cameraRight = normalize(cameraRight);

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
	# pragma endregion

	# pragma region Mouse
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	static vec2	angles = vec2(0, 0);
	angles.x += (mouseX - ((float)WINDOW_WIDTH  / 2)) * CAMERA_SENSITIVITY * window.getFrameTime();
	angles.y -= (mouseY - ((float)WINDOW_HEIGHT / 2)) * CAMERA_SENSITIVITY * window.getFrameTime();
	angles.y = glm::clamp(angles.y, -89.0f, 89.0f);

	vec3	cameraDir = vec3{
		cos(radians(angles.x)) * cos(radians(angles.y)),
		sin(radians(angles.y)),
		sin(radians(angles.x)) * cos(radians(angles.y))
	};

	cameraInfo.lookAt = cameraInfo.position + cameraDir;

	glfwSetCursorPos(window, (float)WINDOW_WIDTH / 2, (float)WINDOW_HEIGHT / 2);
	# pragma endregion
	
	camera.setCameraInfo(cameraInfo);
}

// Handle the inputs for the game
static void inputs(GameData &gameData) {
	VoxelSystem &voxelSystem = gameData.voxelSystem;
	Window 		&window 	 = gameData.window;
	Camera 		&camera 	 = gameData.camera;

	// Delete a chunk, F key
	if (keyPressedOnce(window, GLFW_KEY_F)) {
		vec3 camPos = camera.getCameraInfo().position;
		ivec3 chunkPos = ivec3(
			camPos.z / CHUNK_SIZE,
			camPos.y / CHUNK_SIZE,
			camPos.x / CHUNK_SIZE
		);
		voxelSystem.requestChunk({ChunkRequest{chunkPos, ChunkAction::DELETE}});
		if (VERBOSE)
			cout << "Deleting chunk at worldPos : " << chunkPos.x << " " << chunkPos.y << " " << chunkPos.z << endl;
	}

	// Destroy a block, left click
	if (MouseButtonPressedOnce(window, GLFW_MOUSE_BUTTON_LEFT)) {
		voxelSystem.tryDestroyBlock();
	}
}

// Handle all keyboard & other events
void	handleEvents(GameData &gameData) {
	Window			&window  = gameData.window;
	ShaderHandler	&shaders = gameData.shaders;
	Camera			&camera  = gameData.camera;

	static float time = 20; time += 0.001 * window.getFrameTime(); // Start at early daytime

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// chunkRequests(gameData.voxelSystem, gameData.camera.getCameraInfo());
	cameraMovement(window, camera);
	inputs(gameData);

	// Skybox Shader parameters
	float		dayDuration = 360;
	float		angle = (time / dayDuration) * M_PI;
	vec3		sunPos = normalize(vec3(cos(angle), sin(angle), 0.0f));
	mat4		skyboxView = camera.getProjectionMatrix() * mat4(mat3(camera.getViewMatrix())); // Get rid of the translation part

	shaders.setUniform((*shaders[0])->getID(), "time", time);
	shaders.setUniform((*shaders[0])->getID(), "camera", skyboxView);
	shaders.setUniform((*shaders[0])->getID(), "sunPos", sunPos);

	// Geometrie Pass Shader parameters
	shaders.setUniform((*shaders[1])->getID(), "projection", camera.getProjectionMatrix());
	shaders.setUniform((*shaders[1])->getID(), "view", camera.getViewMatrix());
	shaders.setUniform((*shaders[1])->getID(), "time", time);

	// Lighting Pass Shader Parameters
	shaders.setUniform((*shaders[2])->getID(), "time", time);
	shaders.setUniform((*shaders[2])->getID(), "camera", skyboxView);
	shaders.setUniform((*shaders[2])->getID(), "sunPos", sunPos);
	shaders.setUniform((*shaders[2])->getID(), "gPosition", 0);
	shaders.setUniform((*shaders[2])->getID(), "gNormal", 1);
	shaders.setUniform((*shaders[2])->getID(), "gColor", 2);
	shaders.setUniform((*shaders[2])->getID(), "screenSize", vec2(WINDOW_WIDTH, WINDOW_HEIGHT));
}
