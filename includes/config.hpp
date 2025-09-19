#pragma once

/// Defines
# define COLOR_HEADER_CXX

# define FOV 80.0f
# define WINDOW_WIDTH  1920
# define WINDOW_HEIGHT 1080
# define CAMERA_SPEED  0.006f
# define CAMERA_SPRINT_BOOST  0.05f
# define CAMERA_SENSITIVITY  0.015f

/// System includes
# include <iostream>
# include <fstream>
# include <string.h>
# include <sstream>

/// Framework includes
# include "Window.hpp"
# include "Shader.hpp"
# include "Noise.hpp"
# include "Profiler.hpp"
# include "SkyBox.hpp"
# include "Camera.hpp"
# include "color.h"

/// Custom includes (*.hpp & *.tpp)
# include "VoxelSystem.hpp"

/// Global variables
using namespace std;
using namespace glm;

extern bool VERBOSE;
extern bool SHOW_TOOLTIP;
extern bool POLYGON;

typedef struct {
	GLuint		renderQuadVAO;
} RenderData;

typedef struct GameData {
	Window			&window;
	ShaderHandler	&shaders;
	VoxelSystem		&voxelSystem;
	SkyBox			&skybox;
	Camera			&camera;
	RenderData		&renderDatas;
} GameData;

/// Functions

// flags.cpp
uint64_t	flagHandler(int argc, char **argv);

// rendering.cpp
void	Rendering(Window &window, const uint64_t &seed);

// events.cpp
void	handleEvents(GameData &gameData);

// utils.cpp
void	printControls();
void	printVerbose(const string &message);

