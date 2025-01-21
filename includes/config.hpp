#pragma once

/// Defines
# define COLOR_HEADER_CXX

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

bool	isNum(const std::string &str);

std::vector<std::string>	split(const std::string &str, const char &c);
