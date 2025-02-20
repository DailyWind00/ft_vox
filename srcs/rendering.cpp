# include "config.hpp"
# include <random>

static void	lightingPass(RenderData &datas, GeoFrameBuffers &gBuffer)
{
	// Binding the gBuffer textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gBuffer.gPosition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBuffer.gNormal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gBuffer.gColor);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, datas.ssaoData.ssaoColorBuffer);
	
	// Rendering to the renderQuad
	glDisable(GL_CULL_FACE);
	glBindVertexArray(datas.renderQuadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	glEnable(GL_CULL_FACE);
	
	// Copying the final depth buffer to the default internal framebuffer
	glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.gBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
	glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void	ssaoPass(RenderData &datas, GeoFrameBuffers &gBuffer)
{
	// Binding the gBuffer textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gBuffer.gPosition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBuffer.gNormal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, datas.ssaoData.ssaoNoiseTexture);

	// Rendering to the renderQuad
	glDisable(GL_CULL_FACE);
	glBindVertexArray(datas.ssaoData.ssaoFrameBuffer);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_CULL_FACE);
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

	shaders.use(shaders[3]);
	for (int i = 0; i < 64; i++)
		shaders.setUniform((*shaders[3])->getID(), "samples[" + std::to_string(i) + "]", renderDatas.ssaoData.sampleKernels[i]);
	ssaoPass(renderDatas, gBuffer);

	// Deferred rendering lighting
	shaders.use(shaders[2]);
	lightingPass(renderDatas, gBuffer);

	// Skybox 
	shaders.use(shaders[0]);
	skybox.draw();

	handleEvents(gameData);
	window.setTitle("ft_vox | FPS: " + to_string(window.getFPS()) + " | FrameTime: " + to_string(window.getFrameTime()) + "ms");
}

static float	lerp(float a, float b, float f)
{
	return a + f * (b - a);
}

static std::vector<glm::vec3>	computeSSAOSampleKernel(std::uniform_real_distribution<float> &randomFloats, std::default_random_engine &generator)
{
	std::vector<glm::vec3>			ssaoKernel;

	for (unsigned int i = 0; i < 64; i++) {
    		glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator));

		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		
		// Focus sample on the origin
		float scale = (float)i / 64.0;
		scale = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;

		ssaoKernel.push_back(sample);
	}
	return (ssaoKernel);
}

static std::vector<glm::vec3>	computeSSAONoise(std::uniform_real_distribution<float> &randomFloats, std::default_random_engine &generator)
{
	std::vector<glm::vec3>	ssaoNoise;
	
	for (unsigned int i = 0; i < 16; i++) {
		glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0, 0.0f);
		
		ssaoNoise.push_back(noise);
	} 
	return (ssaoNoise);
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
	Camera 			camera(
		(CameraInfo){{0, 0, 0}, {0, 0, 1}, {0, 1, 0}},
		(ProjectionInfo){FOV, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 10000.0f}
	);
	VoxelSystem		voxelSystem(seed);
	SkyBox			skybox;
	ShaderHandler	shaders; // Skybox -> Voxels Geometrie -> Voxels Lighting
	shaders.add_shader("shaders/Skybox_vert.glsl", "shaders/Skybox_frag.glsl"); // Used by default
	shaders.add_shader("shaders/VoxelGeometrie_vert.glsl", "shaders/VoxelGeometrie_frag.glsl");
	shaders.add_shader("shaders/VoxelLighting_vert.glsl", "shaders/VoxelLighting_frag.glsl");
	shaders.add_shader("shaders/VoxelSSAO_vert.glsl", "shaders/VoxelSSAO_frag.glsl");

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

	/// Setting up SSAO datas
	// random float generator
	std::uniform_real_distribution<float>	randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
	std::default_random_engine		generator;

	// Sample Kernel computation
	renderDatas.ssaoData.sampleKernels = computeSSAOSampleKernel(randomFloats, generator).data();
	std::vector<glm::vec3>	randomVec = computeSSAONoise(randomFloats, generator);
	
	// Noise texture
	glGenTextures(1, &renderDatas.ssaoData.ssaoNoiseTexture);
	glBindTexture(GL_TEXTURE_2D, renderDatas.ssaoData.ssaoNoiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &randomVec[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//
	glGenFramebuffers(1, &renderDatas.ssaoData.ssaoFrameBuffer);
	glBindBuffer(GL_FRAMEBUFFER, renderDatas.ssaoData.ssaoFrameBuffer);
	// Color Buffer
	glGenTextures(1, &renderDatas.ssaoData.ssaoColorBuffer);
	glBindTexture(GL_TEXTURE_2D, renderDatas.ssaoData.ssaoColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RED, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderDatas.ssaoData.ssaoColorBuffer, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "SSAO Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
