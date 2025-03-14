#include "config.hpp"

static void	lightingPass(const GeoFrameBuffers &gBuffer, GLuint &renderQuadVAO) {
	// Binding the gBuffer textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gBuffer.gPosition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBuffer.gNormal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gBuffer.gColor);

	// Rendering to the renderQuad
	glDisable(GL_CULL_FACE);
	glBindVertexArray(renderQuadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	glEnable(GL_CULL_FACE);
	
	// Copying the final depth buffer to the default internal framebuffer
	glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.gBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
	glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Keep the window alive, exiting this function should mean closing the window
static void program_loop(GameData &gameData) {
	Window			&window      = gameData.window;
	ShaderHandler	&shaders     = gameData.shaders;
	VoxelSystem		&voxelSystem = gameData.voxelSystem;
	SkyBox			&skybox      = gameData.skybox;
	RenderData		&renderDatas = gameData.renderDatas;

	// Voxel Geometrie
	shaders.use(shaders[1]);
	GeoFrameBuffers	gBuffer = voxelSystem.draw();

	// Deferred rendering lighting
	shaders.use(shaders[2]);
	lightingPass(gBuffer, renderDatas.renderQuadVAO);

	// Skybox 
	shaders.use(shaders[0]);
	skybox.draw();

	handleEvents(gameData);
	window.setTitle("ft_vox | FPS: " + to_string(window.getFPS()) + " | FrameTime: " + to_string(window.getFrameTime()) + "ms");
}

// Setup variables and call the program loop
void	Rendering(Window &window, const uint64_t &seed) {
	// Mouse Parameters
	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(window, (float)WINDOW_WIDTH / 2, (float)WINDOW_HEIGHT / 2);

	// OpenGL Parameters
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);

	// Systemes Initialization
	Camera	camera(
		(CameraInfo){{0, 0, 0}, {0, 0, 1}, {0, 1, 0}},
		(ProjectionInfo){FOV, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 10000.0f}
	);
	VoxelSystem		voxelSystem(seed, camera);
	SkyBox			skybox;
	ShaderHandler	shaders; // Skybox -> Voxels Geometrie -> Voxels Lighting
	shaders.add_shader("shaders/Skybox_vert.glsl", "shaders/Skybox_frag.glsl"); // Used by default
	shaders.add_shader("shaders/VoxelGeometrie_vert.glsl", "shaders/VoxelGeometrie_frag.glsl");
	shaders.add_shader("shaders/VoxelLighting_vert.glsl", "shaders/VoxelLighting_frag.glsl");

	// Rendering Quad Initialization
	RenderData	renderDatas;
	unsigned int	screenQuadVBO = 0;
	float	screenQuadVert[] = {
		-1, -1, 0, 0, 0,
		-1,  1, 0, 0, 1,
		 1, -1, 0, 1, 0,
		 1,  1, 0, 1, 1
	};

	glGenVertexArrays(1, &renderDatas.renderQuadVAO);
	glBindVertexArray(renderDatas.renderQuadVAO);
	
	glGenBuffers(1, &screenQuadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, screenQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screenQuadVert), screenQuadVert, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	// Setting Game Datas to send to the game loop
	GameData gameData = {
		window,
		shaders,
		voxelSystem,
		skybox,
		camera,
		renderDatas
	};

	window.mainLoop(program_loop, gameData);
}
