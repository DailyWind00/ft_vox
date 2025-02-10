#include "config.hpp"

// Keep the window alive, exiting this function should mean closing the window
static void program_loop(GameData &gameData) {
	Window			&window      = gameData.window;
	ShaderHandler	&shaders     = gameData.shaders;
	VoxelSystem		&voxelSystem = gameData.voxelSystem;
	SkyBox			&skybox      = gameData.skybox;

	shaders.use(shaders[0]); // Use the Skybox shader
	skybox.draw();

	shaders.use(shaders[1]); // Use the Voxel shader
	voxelSystem.draw();

	handleEvents(gameData);
	window.setTitle("ft_vox | FPS: " + std::to_string(window.getFPS()) + " | FrameTime: " + std::to_string(window.getFrameTime()) + "ms");
}

// Setup variables and call the program loop
void	Rendering(Window &window)
{
	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	Camera 			camera(
		(CameraInfo){{0, 0, 0}, {0, 0, 1}, {0, 1, 0}},
		(ProjectionInfo){FOV, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 10000.0f}
	);
	VoxelSystem		voxelSystem(1234);
	SkyBox			skybox;
	ShaderHandler	shaders; // Skybox -> Voxels -> UI
	shaders.add_shader("shaders/Skybox_vert.glsl", "shaders/Skybox_frag.glsl"); // Used by default
	shaders.add_shader("shaders/Voxel_vert.glsl", "shaders/Voxel_frag.glsl");

	GameData gameData = {
		window,
		shaders,
		voxelSystem,
		skybox,
		camera
	};

	window.mainLoop(program_loop, gameData);
}
