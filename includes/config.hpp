#pragma once

/// Defines
# define COLOR_HEADER_CXX

/// System includes
# include <iostream>
# include <fstream>
# include <string.h>

/// Custom includes
# include "WindowsHandler.hpp"
# include "Shader.hpp"
# include "color.h"

/// Global variables
using namespace std;

extern bool VERBOSE;

/// Functions

// utils.cpp
void	printVerbose(const string &message, bool newline = true);