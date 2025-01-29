#pragma once

/// Defines
# define COLOR_HEADER_CXX

# define WINDOW_WIDTH  800
# define WINDOW_HEIGHT 600
# define CAMERA_SPEED  0.1f
# define CAMERA_SPRINT_BOOST  0.15f
# define CAMERA_SENSITIVITY  0.01f

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
# include "color.h"

/// Global variables
using namespace std;

extern bool VERBOSE;

/// Functions
// rendering.cpp

void	Rendering(Window &window);


// events.cpp

void	handleEvents(Window &window, Shader &shader);


// utils.cpp

void	printVerbose(const string &message);

bool	isNum(const std::string &str);

std::vector<std::string>	split(const std::string &str, const char &c);
