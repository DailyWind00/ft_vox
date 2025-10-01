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

static bool isColliding(Player &player, VoxelSystem &voxelSystem) {
	if (!player.HaveNoclip()) {
		AABB box = player.getCameraBox();

		array<vec3, 8> corners = {
			vec3(box.min.x, box.min.y, box.min.z),
			vec3(box.max.x, box.min.y, box.min.z),
			vec3(box.min.x, box.max.y, box.min.z),
			vec3(box.max.x, box.max.y, box.min.z),
			vec3(box.min.x, box.min.y, box.max.z),
			vec3(box.max.x, box.min.y, box.max.z),
			vec3(box.min.x, box.max.y, box.max.z),
			vec3(box.max.x, box.max.y, box.max.z)
		};
		for (const auto& corner : corners) {
			uint8_t block = voxelSystem.getBlockAt(floor(corner));
			if (block != 0 && block != WATER) // Skip air and water
				return true;
		}
	}
	return false;
}

static bool playerInWater(Player &player, VoxelSystem &voxelSystem) {
	AABB box = player.getCameraBox();

	array<vec3, 8> corners = {
		vec3(box.min.x, box.min.y, box.min.z),
		vec3(box.max.x, box.min.y, box.min.z),
		vec3(box.min.x, box.max.y, box.min.z),
		vec3(box.max.x, box.max.y, box.min.z),
		vec3(box.min.x, box.min.y, box.max.z),
		vec3(box.max.x, box.min.y, box.max.z),
		vec3(box.min.x, box.max.y, box.max.z),
		vec3(box.max.x, box.max.y, box.max.z)
	};
	for (const auto& corner : corners) {
		uint8_t block = voxelSystem.getBlockAt(floor(corner));
		if (block == WATER) // Water block
			return true;
	}
	return false;
}

// Handle the camera movements/interactions
static void	cameraMovement(GameData &gameData) {
	Player		&player = gameData.player;
	Window		&window = gameData.window;
	VoxelSystem	&voxelSystem = gameData.voxelSystem;
	CameraInfo	cameraInfo = player.getCamera().getCameraInfo();

	# pragma region Camera controls

	vec3	cameraFront = cameraInfo.lookAt - cameraInfo.position;
	vec3	cameraRight = normalize(cross(cameraFront, cameraInfo.up));
	cameraFront.y = 0; cameraRight.y = 0; // Remove the Y axis
	cameraFront = normalize(cameraFront);
	cameraRight = normalize(cameraRight);

	const float camSpeed = (CAMERA_SPEED + (CAMERA_SPRINT_BOOST * player.IsSprinting())) * window.getFrameTime();

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		player.setSprinting(true);

	vec3 move = vec3(0);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) move += cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) move -= cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) move -= cameraRight;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) move += cameraRight;
	if (!player.HaveNoclip() && (player.CanJump() || playerInWater(player, voxelSystem)) && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		player.setCanJump(false);
		player.setIsFalling(false);
		player.setGravity(GRAVITY_MAX);
	}
	else if (player.HaveNoclip() && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		move += cameraInfo.up;
	if ((player.HaveNoclip() || playerInWater(player, voxelSystem)) && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		move -= cameraInfo.up;

	if (length(move) == 0.0f) // No movement
		player.setSprinting(false);

	if (length(move) > 1.0f)
		move = normalize(move);

	vec3 translation = move * camSpeed;

	if (!player.HaveNoclip()) translation *= 0.25f; // 75% slower when not in noclip mode  
	if (!player.HaveNoclip()) translation.y += player.getGravity();
	if (playerInWater(player, voxelSystem)) {
		translation.x *= 0.40f; // 60% slower in water
		translation.y *= 0.20f; // 80% slower in water on Y axis
		translation.z *= 0.40f; // 60% slower in water
	}

	bool hasCollided = false;

	// Try move on Y
	player.translate(vec3(0, translation.y, 0));
	if (isColliding(player, voxelSystem)) {
		player.translate(vec3(0, -translation.y, 0));
		hasCollided = true;
		player.setGravity(0.0f); // landed
	}
	// Try move on X
	player.translate(vec3(translation.x, 0, 0));
	if (isColliding(player, voxelSystem)) {
		player.translate(vec3(-translation.x, 0, 0));
		hasCollided = true;
	}
	// Try move on Z
	player.translate(vec3(0, 0, translation.z));
	if (isColliding(player, voxelSystem)) {
		player.translate(vec3(0, 0, -translation.z));
		hasCollided = true;
	}
	
	// Gravity handling
	if (!player.HaveNoclip()) {
		if (playerInWater(player, voxelSystem)) { // In water
			player.setCanJump(true);
			player.setIsFalling(true);
		}
		else if (!hasCollided) { // Falling
			player.setCanJump(false);
			player.setIsFalling(true);
		}
		else if (hasCollided && !playerInWater(player, voxelSystem)) { // Landed
			player.setCanJump(true);
			player.setIsFalling(false);
			player.setGravity(0.0f);
		}
	}

	# pragma endregion

	cameraInfo = player.getCamera().getCameraInfo(); // Refresh camera info after potential movement

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

	glfwSetCursorPos(window, (float)WINDOW_WIDTH / 2, (float)WINDOW_HEIGHT / 2);

	player.getCamera().setLookAt(cameraInfo.position + cameraDir);

	# pragma endregion
}

// Handle the inputs for the game
static void inputs(GameData &gameData) {
	VoxelSystem 	&voxelSystem = gameData.voxelSystem;
	Window 			&window 	 = gameData.window;
	ShaderHandler	&shaders	 = gameData.shaders;
	Player			&player		 = gameData.player;

	// Close the window
	if (keyPressedOnce(window, GLFW_KEY_ESCAPE))
		glfwSetWindowShouldClose(window, true);

	// Recompile shaders
	if (keyPressedOnce(window, GLFW_KEY_P)) {
		cout << "Recompiling shaders...\n";
		(*shaders[0])->recompile();
		(*shaders[1])->recompile();
		(*shaders[2])->recompile();
		(*shaders[3])->recompile();
		(*shaders[4])->recompile();
		(*shaders[5])->recompile();
		cout << "Shaders recompiled\n";
	}

	// Destroy a block
	if (MouseButtonPressedOnce(window, GLFW_MOUSE_BUTTON_LEFT))
		voxelSystem.tryDestroyBlock();

	// Flashlight
	if (keyPressedOnce(window, GLFW_KEY_F)) {
		player.setFlashlight(!player.HaveFlashlight());
		if (VERBOSE)
			cout << "Flashlight " << (player.HaveFlashlight() ? "ON" : "OFF") << endl;
	}

	// Noclip mode
	if (keyPressedOnce(window, GLFW_KEY_N)) {
		player.setNoclip(!player.HaveNoclip());
		player.setCanJump(false);
		player.setIsFalling(true);
		player.setGravity(0.0f);
		if (VERBOSE)
			cout << "Noclip mode " << (player.HaveNoclip() ? "ON" : "OFF") << endl;
	}

	// Polygon mode
	if (keyPressedOnce(window, GLFW_KEY_L)) {
		POLYGON = !POLYGON;
		if (VERBOSE)
			cout << "Polygon mode " << (POLYGON ? "ON" : "OFF") << endl;
	}
}

// Add condition to the function VoxelSystem::loadChunksAroundCamera to avoid calling it every frame
static void dynamicChunkLoading(VoxelSystem &voxelSystem, const CameraInfo &camInfo) {
	// Camera data initialization
	ivec3		chunkPos = {
		camInfo.position.x / CHUNK_SIZE,
		camInfo.position.y / CHUNK_SIZE,
		camInfo.position.z / CHUNK_SIZE
	};
	static ivec3	lastChunkPos = vec3(chunkPos);

	// Request distance data initialization
	static int		horRequestDistance = glm::min(SPAWN_LOCATION_SIZE, HORIZONTAL_RENDER_DISTANCE);
	static int		vertRequestDistance = glm::min(SPAWN_LOCATION_SIZE, VERTICAL_RENDER_DISTANCE);
	list<ChunkRequest>	chunkRequests;

	// Only requests new chunks if Threads are free
	if (voxelSystem.getChunkRequestCount() != 0)
		return ;

	horRequestDistance = glm::clamp(horRequestDistance, 0, HORIZONTAL_RENDER_DISTANCE - 1);
	vertRequestDistance = glm::clamp(vertRequestDistance, 0, VERTICAL_RENDER_DISTANCE);
	if (chunkPos == lastChunkPos) {
		if (horRequestDistance >= HORIZONTAL_RENDER_DISTANCE || vertRequestDistance >= VERTICAL_RENDER_DISTANCE)
			return ;
		for (int i = -horRequestDistance; i <= horRequestDistance; i++) {
			for (int j = -vertRequestDistance; j < vertRequestDistance; j++) {
				chunkRequests.push_back({{i + chunkPos.x, j + chunkPos.y, -horRequestDistance + chunkPos.z}, ChunkAction::CREATE_UPDATE});
				chunkRequests.push_back({{i + chunkPos.x, j + chunkPos.y, horRequestDistance + chunkPos.z}, ChunkAction::CREATE_UPDATE});
				chunkRequests.push_back({{-horRequestDistance + chunkPos.x, j + chunkPos.y, i + chunkPos.z}, ChunkAction::CREATE_UPDATE});
				chunkRequests.push_back({{horRequestDistance + chunkPos.x, j + chunkPos.y, i + chunkPos.z}, ChunkAction::CREATE_UPDATE});
			}
		}
		for (int i = -horRequestDistance; i <= horRequestDistance; i++) {
			for (int j = -horRequestDistance; j < horRequestDistance; j++) {
				chunkRequests.push_back({{i + chunkPos.x, -vertRequestDistance + chunkPos.y, j + chunkPos.z}, ChunkAction::CREATE_UPDATE});
				chunkRequests.push_back({{i + chunkPos.x, vertRequestDistance + chunkPos.y, j + chunkPos.z}, ChunkAction::CREATE_UPDATE});
			}
		}
		horRequestDistance++;
		vertRequestDistance++;
	}
	else {
		horRequestDistance -= abs(lastChunkPos.y - chunkPos.y) + abs(lastChunkPos.x - chunkPos.x) + abs(lastChunkPos.z - chunkPos.z);
		vertRequestDistance -= abs(lastChunkPos.y - chunkPos.y) + abs(lastChunkPos.x - chunkPos.x) + abs(lastChunkPos.z - chunkPos.z);
	}
	
	// Updating last chunk position to current

	// Sort the requests by distance to the camera, to load the closest chunks first
	chunkRequests.sort(
		[&chunkPos](const ChunkRequest &a, const ChunkRequest &b) {
			vec3 da = (vec3)a.first - (vec3)chunkPos;
			vec3 db = (vec3)b.first - (vec3)chunkPos;
			return dot(da, da) < dot(db, db);
		});


	// Searching for chunk to delete and send all the requests
	voxelSystem.findChunksToDelete(chunkRequests);
	voxelSystem.requestChunk(chunkRequests);
	lastChunkPos = chunkPos;
}

// Handle all keyboard & other events
void	handleEvents(GameData &gameData) {
	Window			&window  = gameData.window;
	ShaderHandler	&shaders = gameData.shaders;
	Player			&player  = gameData.player;
	Camera			&camera  = player.getCamera();
	Camera			&shadowMapCam = gameData.shadowMapCam;

	static float	time = 20; // Start at early daytime

	// Simulation pause if the window is not focused
	if (window.isFocused()) {
		time += 0.001 * window.getFrameTime();

		// Gravity update
		if (!player.HaveNoclip() && player.IsFalling()) {
			float targetGravity = glm::clamp(
				(float)(player.getGravity() - GRAVITY_STRENGTH * window.getFrameTime()),
				GRAVITY_MIN,
				GRAVITY_MAX
			);

			float smoothing = 10.0f;
			float newGravity = glm::mix(
				player.getGravity(), // current
				targetGravity,       // target
				1.0f - exp(-smoothing * window.getFrameTime()) // framerate independent lerp
			);

			player.setGravity(newGravity);
		}

		inputs(gameData);
		cameraMovement(gameData);
		dynamicChunkLoading(gameData.voxelSystem, camera.getCameraInfo());
	}

	// Skybox Shader parameters
	float	dayDuration = 10;
	vec2	angles = {(time / dayDuration) * M_PI, 25.0f};
	vec3	sunPos {
		cos(radians(angles.x)) * cos(radians(angles.y)),
		sin(radians(angles.x)) * cos(radians(angles.y)),
		sin(radians(angles.y)),
	};

	mat4		skyboxView = camera.getProjectionMatrix() * mat4(mat3(camera.getViewMatrix())); // Get rid of the translation part
	vec3		camPos = camera.getCameraInfo().position;
	bool		inWater = (gameData.voxelSystem.getBlockAt(camPos) == 9) ? true : false;

	shadowMapCam.setPosition({camPos.x + sunPos.x * 600, camPos.y + sunPos.y * 600, camPos.z + sunPos.z * 600});
	shadowMapCam.setLookAt(camPos);

	// Skybox Pass Shader Parameters
	shaders.setUniform((*shaders[0])->getID(), "time", time);
	shaders.setUniform((*shaders[0])->getID(), "camera", skyboxView);
	shaders.setUniform((*shaders[0])->getID(), "sunPos", sunPos);

	// Geometrie Pass Shader parameters
	shaders.setUniform((*shaders[1])->getID(), "time", time);
	shaders.setUniform((*shaders[1])->getID(), "sunPos", sunPos);
	shaders.setUniform((*shaders[1])->getID(), "projection", camera.getProjectionMatrix());
	shaders.setUniform((*shaders[1])->getID(), "view", camera.getViewMatrix());
	shaders.setUniform((*shaders[1])->getID(), "polygonVisible", POLYGON);
	shaders.setUniform((*shaders[1])->getID(), "camPos", camPos);

	// Lighting Pass Shader Parameters
	shaders.setUniform((*shaders[2])->getID(), "camPos", camPos);
	shaders.setUniform((*shaders[2])->getID(), "renderDistance", (float)HORIZONTAL_RENDER_DISTANCE);
	shaders.setUniform((*shaders[2])->getID(), "inWater", inWater);
	shaders.setUniform((*shaders[2])->getID(), "flashlightOn", player.HaveFlashlight());
	shaders.setUniform((*shaders[2])->getID(), "spView", camera.getViewMatrix());
	shaders.setUniform((*shaders[2])->getID(), "spProj", camera.getProjectionMatrix());
	shaders.setUniform((*shaders[2])->getID(), "lpMat", shadowMapCam.getProjectionMatrix() * shadowMapCam.getViewMatrix());
	shaders.setUniform((*shaders[2])->getID(), "camera", skyboxView);
	shaders.setUniform((*shaders[2])->getID(), "sunPos", sunPos);
	shaders.setUniform((*shaders[2])->getID(), "gPosition", 0);
	shaders.setUniform((*shaders[2])->getID(), "gNormal", 1);
	shaders.setUniform((*shaders[2])->getID(), "gColor", 2);
	shaders.setUniform((*shaders[2])->getID(), "gEmissive", 3);
	shaders.setUniform((*shaders[2])->getID(), "shadowMap", 4);
	shaders.setUniform((*shaders[2])->getID(), "screenSize", vec2(WINDOW_WIDTH, WINDOW_HEIGHT));

	// Shadow Mapping Pass Shader parameters
	shaders.setUniform((*shaders[3])->getID(), "projection", shadowMapCam.getProjectionMatrix());
	shaders.setUniform((*shaders[3])->getID(), "view", shadowMapCam.getViewMatrix());

	// Post Processing Pass Shader Parameters
	shaders.setUniform((*shaders[4])->getID(), "time", time);
	shaders.setUniform((*shaders[4])->getID(), "postProcBuffer", 0);
	shaders.setUniform((*shaders[4])->getID(), "depthBuffer", 1);
	shaders.setUniform((*shaders[4])->getID(), "test3D", 2);
	shaders.setUniform((*shaders[4])->getID(), "view", camera.getViewMatrix());
	shaders.setUniform((*shaders[4])->getID(), "sunPos", sunPos);
	shaders.setUniform((*shaders[4])->getID(), "camPos", camPos);
	shaders.setUniform((*shaders[4])->getID(), "camDir", camera.getCameraInfo().lookAt - camera.getCameraInfo().position);
	shaders.setUniform((*shaders[4])->getID(), "screenSize", vec2(WINDOW_WIDTH, WINDOW_HEIGHT));

	// Bounding box (for debug)
	if (POLYGON) {
		shaders.setUniform((*shaders[5])->getID(), "view", camera.getViewMatrix());
		shaders.setUniform((*shaders[5])->getID(), "projection", camera.getProjectionMatrix());
	}
}