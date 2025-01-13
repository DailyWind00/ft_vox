#pragma once

/// Defines
# define COLOR_HEADER_CXX

/// System includes
# include <iostream>
# include <fstream>
# include <string.h>

/// Custom includes (*.hpp & *.tpp)
# include "Window.hpp"
# include "Shader.hpp"
# include "Profiler.hpp"
# include "VoxelSystem.tpp"
# include "color.h"

/// Global variables
using namespace std;

extern bool VERBOSE;

/// Functions
// rendering.cpp

void	Rendering(Window &window);


// events.cpp

void	handleEvents(Window &window);


// utils.cpp

void	printVerbose(const string &message);
