#pragma once

/// Defines
# define COLOR_HEADER_CXX

# define FOV 80.0f
# define WINDOW_WIDTH  1920
# define WINDOW_HEIGHT 1080
# define CAMERA_SPEED  0.02f
# define CAMERA_SPRINT_BOOST  0.05f
# define CAMERA_SENSITIVITY  0.035f

/// System includes
# include <iostream>
# include <fstream>
# include <string.h>
# include <sstream>

/// Custom includes (*.hpp & *.tpp)
# include "Window.hpp"
# include "Shader.hpp"
# include "Noise.hpp"
# include "FlagHandler.hpp"
# include "Profiler.hpp"
# include "VoxelSystem.hpp"
# include "SkyBox.hpp"
# include "Camera.hpp"
# include "color.h"

/// Global variables
using namespace std;

typedef struct GeoFrameBuffers {
	GLuint	gBuffer;
	GLuint	gPosition;
	GLuint	gNormal;
	GLuint	gColor;
}	GeoFrameBuffers;

typedef struct {
	GLuint		renderQuadVAO;
}	RenderData;

typedef struct GameData {
	Window		&window;
	ShaderHandler	&shaders;
	VoxelSystem	&voxelSystem;
	SkyBox		&skybox;
	Camera		&camera;
	RenderData	&renderDatas;
} GameData;

extern bool VERBOSE;

/// Functions

// rendering.cpp

void	Rendering(Window &window);

// events.cpp

void	handleEvents(GameData &gameData);

// utils.cpp

void	printVerbose(const string &message);

