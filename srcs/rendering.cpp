#include "config.hpp"

static void	lightingPass(const ShadowMappingData &shadowMapData, const GeoFrameBuffers &gBuffer, const PostProcessingData &postProcData, GLuint &renderQuadVAO) {
	// Binding the gBuffer textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gBuffer.gPosition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBuffer.gNormal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gBuffer.gColor);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gBuffer.gEmissive);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, shadowMapData.depthMap);

	// Rendering to the renderQuad
	glDisable(GL_CULL_FACE);

	glBindFramebuffer(GL_FRAMEBUFFER, postProcData.postProcFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(renderQuadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glEnable(GL_CULL_FACE);
	
	// Copying the final depth buffer to the default internal framebuffer
	glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.gBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postProcData.postProcFBO); // write to post processing framebuffer
	glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void	postProcessingPass(const PostProcessingData &postProcData, const GLuint &cloudSampleTexture, GLuint &renderQuadVAO) {
	// Binding the post processing texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, postProcData.postProcBuffer);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, postProcData.postProcDepthBuffer);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_3D, cloudSampleTexture);

	// Rendering to the renderQuad
	glDisable(GL_CULL_FACE);
	glBindVertexArray(renderQuadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	glEnable(GL_CULL_FACE);

	// Copying the final depth buffer to the default internal framebuffer
	glBindFramebuffer(GL_READ_FRAMEBUFFER, postProcData.postProcBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to post processing framebuffer
	glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Keep the window alive, exiting this function should mean closing the window
static void program_loop(GameData &gameData) {
	static Window		&window      = gameData.window;
	static ShaderHandler	&shaders     = gameData.shaders;
	static VoxelSystem	&voxelSystem = gameData.voxelSystem;
	static SkyBox		&skybox      = gameData.skybox;
	static RenderData	&renderDatas = gameData.renderDatas;

	PostProcessingData	postProcData = voxelSystem.getPostProcData();

	shaders.use(shaders[3]);
	glViewport(0, 0, SHADOW_RESOLUTION, SHADOW_RESOLUTION);
	ShadowMappingData	shadowMapData = voxelSystem.renderShadowMapPass(shaders);

	// Voxel Geometrie
	shaders.use(shaders[1]);
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	GeoFrameBuffers	gBuffer = voxelSystem.renderGeometryPass(shaders);

	// Deferred rendering lighting
	shaders.use(shaders[2]);
	lightingPass(shadowMapData, gBuffer, postProcData, renderDatas.renderQuadVAO);

	// Skybox 
	shaders.use(shaders[0]);
	glBindFramebuffer(GL_FRAMEBUFFER, postProcData.postProcFBO);
	skybox.draw();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	shaders.use(shaders[4]);
	postProcessingPass(postProcData, gameData.cloudSytem.getCloudNoiseSample(), renderDatas.renderQuadVAO);

	handleEvents(gameData);
	window.setTitle("ft_vox | FPS: " + to_string(window.getFPS()) + " | FrameTime: " + to_string(window.getFrameTime()) + "ms");
}

static RenderData	initScreenQuad() {
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
	return renderDatas;
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
		(CameraInfo){{0, 0, -3.0}, {0, 0, 1}, {0, 1, 0}},
		(ProjectionInfo){FOV, {(float)WINDOW_WIDTH, (float)WINDOW_HEIGHT}, {0.0f, 0.0f}, 0.1f, 10000.0f},
		ProjectionType::PERSPECTIVE
	);
	Camera	shadowMapCam(
		(CameraInfo){{100, 100, 0}, {0, 0, 0}, {0, 1, 0}},
		(ProjectionInfo){FOV, {SHADOW_FRUSTUM_SIZE / 2, SHADOW_FRUSTUM_SIZE / 2}, {-SHADOW_FRUSTUM_SIZE / 2, -SHADOW_FRUSTUM_SIZE / 2}, 0.1f, 1200.0f},
		ProjectionType::ORTHOGRAPHIC
	);
	VoxelSystem		voxelSystem(seed, camera, shadowMapCam);
	SkyBox			skybox;
	ShaderHandler	shaders; // Skybox -> Voxels Geometrie -> Voxels Lighting
	shaders.add_shader("shaders/Skybox_vert.glsl", "shaders/Skybox_frag.glsl"); // Used by default
	shaders.add_shader("shaders/VoxelGeometrie_vert.glsl", "shaders/VoxelGeometrie_frag.glsl");
	shaders.add_shader("shaders/VoxelLighting_vert.glsl", "shaders/VoxelLighting_frag.glsl");
	shaders.add_shader("shaders/ShadowMap_vert.glsl", "shaders/ShadowMap_frag.glsl");
	shaders.add_shader("shaders/PostProcessing_vert.glsl", "shaders/PostProcessing_frag.glsl");
	RenderData	renderDatas = initScreenQuad();
	CloudSystem	cloudSystem(2048);

	// Setting Game Datas to send to the game loop
	GameData gameData = {
		window,
		shaders,
		voxelSystem,
		cloudSystem,
		skybox,
		camera,
		shadowMapCam,
		renderDatas
	};

	window.mainLoop(program_loop, gameData);
}
