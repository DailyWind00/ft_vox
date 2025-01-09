/// Class independant system includes
# include <iostream>
# include <algorithm>
# include <sstream>
# include <fstream>

# include "Shader.hpp"
# include "color.h"

//// Shader class
/// Constructors & Destructors
Shader::Shader(const std::string &vertexPath, const std::string &fragmentPath, const std::string &geometryPath) {
	if (VERBOSE)
		std::cout << "Creating shader\n";

	this->vertexPath = vertexPath;
	this->fragmentPath = fragmentPath;
	this->geometryPath = geometryPath;

	shaderID = make_shader();

	if (VERBOSE)
		std::cout << "Shader created\n";
}

Shader::~Shader() {
	glUseProgram(0);
	glDeleteProgram(shaderID);

	if (VERBOSE)
		std::cout << "Shader deleted\n";
}
/// ---



/// Privates functions

// Create a shader module from a file
GLuint Shader::make_module(const std::string &filepath, GLuint module_type) {
	std::ifstream file(filepath);
	std::stringstream buffer;
	std::string line;

	if (VERBOSE)
		std::cout << "> Compiling shader : " << filepath << " -> ";

	if (!file.is_open()) {
		if (VERBOSE)
			std::cout << BRed << "Error\n" << ResetColor;
		throw std::runtime_error("Failed to open file " + filepath);
	}

	while (getline(file, line))
		buffer << line << '\n';
	file.close();

	std::string shaderSourceStr = buffer.str();			// Cause corruption if
	const char *shaderSource = shaderSourceStr.c_str(); // in a single line

	GLuint shaderModule = glCreateShader(module_type);
	glShaderSource(shaderModule, 1, &shaderSource, nullptr);
	glCompileShader(shaderModule);

	int success;
	glGetShaderiv(shaderModule, GL_COMPILE_STATUS, &success);
	if (!success) {
		if (VERBOSE)
			std::cout << BRed << "Error\n" << ResetColor;
		std::string infoLog;
		infoLog.resize(1024);
		glGetShaderInfoLog(shaderModule, 1024, nullptr, (GLchar *)infoLog.data());
		throw std::runtime_error("Failed to compile shader " + filepath + ":\n" + infoLog);
	}

	if (VERBOSE)
		std::cout << BGreen << "Shader compiled\n" << ResetColor;

	return shaderModule;
}

// Create a shader from the modules
GLuint Shader::make_shader() {
	GLuint shadersIDs[3];

	shadersIDs[0] = make_module(vertexPath, GL_VERTEX_SHADER);
	shadersIDs[1] = make_module(fragmentPath, GL_FRAGMENT_SHADER);
	if (!geometryPath.empty())
		shadersIDs[2] = make_module(geometryPath, GL_GEOMETRY_SHADER);

	if (VERBOSE)
		std::cout << "> Linking shader -> ";

	GLuint shader = glCreateProgram();
	for (GLuint module = 0; module < (!geometryPath.empty() ? 2 : 3); module++) {
		glAttachShader(shader, shadersIDs[module]);
		glDeleteShader(shadersIDs[module]);
	}

	glLinkProgram(shader);

	int success;
	glGetProgramiv(shader, GL_LINK_STATUS, &success);
	if (!success) {
		if (VERBOSE)
			std::cout << BRed << "Error\n" << ResetColor;
		std::string infoLog;
		infoLog.resize(1024);
		glGetProgramInfoLog(shader, 1024, nullptr, (GLchar *)infoLog.data());
		throw std::runtime_error("Failed to link shader:\n" + infoLog);
	}

	if (VERBOSE)
		std::cout << BGreen << "Shader linked\n" << ResetColor;

	return shader;
}

/// ---



/// Public functions

// Set this shader as the current shader
void Shader::use() {
	glUseProgram(shaderID);

	if (VERBOSE)
		std::cout << "Shader " << shaderID << " in use\n";
}

// Recompile the shader
void Shader::recompile() {
	if (VERBOSE)
		std::cout << "Recompiling shader " << shaderID << " ...\n";

	glUseProgram(0);
	glDeleteProgram(shaderID);

	try {
		shaderID = make_shader();
	}
	catch (const std::exception &e) {
		std::cerr << BRed << "Shader recompilation error : " << e.what() << ResetColor << std::endl;
		return;
	}

	if (VERBOSE)
		std::cout << "Shader recompiled\n";
}

/// ---



/// Uniforms setters

// Set a boolean uniform
void Shader::setUniform(const std::string &name, bool value) {
	glUniform1i(glGetUniformLocation(shaderID, name.c_str()), (int)value);
}

// Set an integer uniform
void Shader::setUniform(const std::string &name, int value) {
	glUniform1i(glGetUniformLocation(shaderID, name.c_str()), value);
}

// Set a float uniform
void Shader::setUniform(const std::string &name, float value) {
	glUniform1f(glGetUniformLocation(shaderID, name.c_str()), value);
}

// Set a vec2 uniform
void Shader::setUniform(const std::string &name, glm::vec2 value) {
	glUniform2f(glGetUniformLocation(shaderID, name.c_str()), value[0], value[1]);
}

// Set a vec3 uniform
void Shader::setUniform(const std::string &name, glm::vec3 value) {
	glUniform3f(glGetUniformLocation(shaderID, name.c_str()), value[0], value[1], value[2]);
}

// Set a vec4 uniform
void Shader::setUniform(const std::string &name, glm::vec4 value) {
	glUniform4f(glGetUniformLocation(shaderID, name.c_str()), value[0], value[1], value[2], value[3]);
}

// Set a mat4 uniform
void Shader::setUniform(const std::string &name, glm::mat4 value) {
	glUniformMatrix4fv(glGetUniformLocation(shaderID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}
/// ---



/// Getters

// Return the shader ID
const GLuint &Shader::getID() const {
	return shaderID;
}
/// ---
//// ----



























// /// Constructors & Destructors 
// Shader::Shader() {
// 	if (VERBOSE)
// 		std::cout << "Shader class created\n";
// }

// Shader::~Shader() {
// 	glUseProgram(0);

// 	ShaderIterator	it;
// 	while (!shaders.empty()) {
// 		it = shaders.begin();
// 		remove_shader(it->first);
// 	}
// 	shaders.clear();
// 	if (VERBOSE)
// 		std::cout << "Shader class deleted\n";
// }
// /// ---



// /// Privates functions

// // Should never be called outside of make_shader()
// GLuint Shader::make_module(const std::string &filepath, GLuint module_type) {
// 	std::ifstream file(filepath);
// 	std::stringstream buffer;
// 	std::string line;

// 	if (VERBOSE)
// 		std::cout << "> Compiling shader : " << filepath << " -> ";

// 	if (!file.is_open()) {
// 		if (VERBOSE)
// 			std::cout << BRed << "Error" << ResetColor;
// 		throw std::runtime_error("Failed to open file " + filepath);
// 	}

// 	while (getline(file, line))
// 		buffer << line << '\n';

// 	std::string shaderSourceStr = buffer.str();				// Cause corruption if 
// 	const char *shaderSource = shaderSourceStr.c_str(); // in a single line
// 	file.close();

// 	GLuint shaderModule = glCreateShader(module_type);
// 	glShaderSource(shaderModule, 1, &shaderSource, nullptr);
// 	glCompileShader(shaderModule);

// 	int success;
// 	glGetShaderiv(shaderModule, GL_COMPILE_STATUS, &success);
// 	if (!success) {
// 		if (VERBOSE)
// 			std::cout << BRed << "Error" << ResetColor;
// 		std::string infoLog;
// 		infoLog.resize(1024);
// 		glGetShaderInfoLog(shaderModule, 1024, nullptr, (GLchar *)infoLog.data());
// 		throw std::runtime_error("Failed to compile shader " + filepath + ":\n\t" + infoLog);
// 	}

// 	if (VERBOSE)
// 		std::cout << BGreen << "Shader compiled" << ResetColor;

// 	return shaderModule;
// }

// GLuint Shader::make_shader(const std::string &vertex_path, const std::string &fragment_path) {
// 	std::vector<GLuint> modules;
// 	modules.push_back(make_module(vertex_path, GL_VERTEX_SHADER));
// 	modules.push_back(make_module(fragment_path, GL_FRAGMENT_SHADER));

// 	GLuint shader = glCreateProgram();
// 	for (GLuint module : modules) {
// 		glAttachShader(shader, module);
// 		glDeleteShader(module);
// 	}

// 	if (VERBOSE)
// 		std::cout << "> Linking shader -> ";

// 	glLinkProgram(shader);

// 	int success;
// 	glGetProgramiv(shader, GL_LINK_STATUS, &success);
// 	if (!success) {
// 		if (VERBOSE)
// 			std::cout << BRed << "Error" << ResetColor;
// 		std::string infoLog;
// 		infoLog.resize(1024);
// 		glGetProgramInfoLog(shader, 1024, nullptr, (GLchar *)infoLog.data());
// 		throw std::runtime_error("Failed to link shader:\n\t" + infoLog);
// 	}

// 	if (VERBOSE)
// 		std::cout << BGreen << "Shader linked" << ResetColor;

// 	return shader;
// }
// /// ---



// /// Public functions

// // Use the shader given with the shaderID,
// void Shader::use(GLuint shaderID) {
//     if (shaders.empty()) {
//         std::cerr << BRed << "Error: No shaders available to use." << ResetColor << std::endl;
//         return;
//     }

//     auto it = shaders.find(shaderID);
//     if (it == shaders.end()) {
//         std::cerr << BRed << "Error: Shader ID " << shaderID << " not found." << ResetColor << std::endl;
//         return;
//     }

//     glUseProgram(it->first);
// 	currentShaderID = it->first;
// }

// // Recompile the shader given with the shaderID
// // Return the id of the new recompiled shader
// GLuint	Shader::recompile(GLuint shaderID) {
// 	ShaderIterator	shader = shaders.find(shaderID);
// 	if (shader == shaders.end()) {
// 		std::cerr << BRed << "Shader recompilation error : Shader ID " << shaderID << " not found." << ResetColor << std::endl;
// 		return 0;
// 	}

// 	if (VERBOSE)
// 		std::cout << "Recompiling shader \"" << shader->second.shaderName << "\" ..." << std::endl;

// 	glUseProgram(0);

// 	GLuint newID = 0;
// 	try {
// 		newID = make_shader(shader->second.vertexPath, shader->second.fragmentpath);
// 		shaders.insert(ShaderPair(newID, shader->second));
// 		remove_shader(shaderID);
// 		currentShaderID = newID;
// 		glUseProgram(currentShaderID);
// 	}
// 	catch(const std::exception& e) {
// 		std::cerr << BRed << "Shader recompilation error : " << e.what() << ResetColor <<  std::endl;
// 		glUseProgram(currentShaderID);
// 		return 0;
// 	}

// 	if (VERBOSE)
// 		std::cout << "Recompilation done" << std::endl;

// 	return newID;
// }

// // Add a new shader to the Shaders class, return the id of the new shader
// // By default, the first shader added is the current shader
// GLuint	Shader::add_shader(const std::string &vertexPath, const std::string &fragmentPath, const std::string &shaderName) {
// 	if (VERBOSE)
// 		std::cout << "Creating shader \"" << shaderName << "\"" << std::endl;

// 	shaderData	data = {
// 		make_shader(vertexPath, fragmentPath),
// 		vertexPath,
// 		fragmentPath,
// 		shaderName
// 	};
// 	shaders.insert(ShaderPair(data.shaderID, data));
// 	shaderIDs.push_back(data.shaderID);

// 	if (VERBOSE)
// 		std::cout << "Shader \"" << shaderName << "\" created with ID " << data.shaderID << std::endl;

// 	if (shaders.size() == 1)
// 		currentShaderID = data.shaderID;
// 	if (shaders.size() == 1 || currentShaderID == 0)
// 		glUseProgram(data.shaderID);

// 	return data.shaderID;
// }

// // Remove a shader from the Shaders class
// void	Shader::remove_shader(GLuint shaderID) {
// 	glDeleteProgram(shaderID);
// 	shaderIDs.erase(remove(shaderIDs.begin(), shaderIDs.end(), shaderID), shaderIDs.end());
// 	shaders.erase(shaderID);
// }

// // Use the next shader in the vector as the current shader
// // Return the id of the new current shader in the vector
// GLuint	Shader::SetNextShader() {
// 	if (shaders.empty()) {
// 		std::cerr << BRed << "Error: No shaders available to use." << ResetColor << std::endl;
// 		return 0;
// 	}

// 	auto	it = shaders.find(currentShaderID);
// 	if (it == shaders.end()) {
// 		std::cerr << BRed << "Error: Shader ID " << currentShaderID << " not found." << ResetColor << std::endl;
// 		return 0;
// 	}
// 	if (++it == shaders.end())
// 		it = shaders.begin();

// 	if (VERBOSE)
// 		std::cout << "Now using shader " << it->first << " \"" << it->second.shaderName << "\"" << std::endl;
	
// 	currentShaderID = it->first;
// 	glUseProgram(currentShaderID);

// 	return currentShaderID;
// }

// // Use the previous shader in the vector as the current shader
// // Return the id of the new current shader in the vector
// GLuint	Shader::SetPreviousShader() {
// 	if (shaders.empty()) {
// 		std::cerr << BRed << "Error: No shaders available to use." << ResetColor << std::endl;
// 		return 0;
// 	}

// 	auto	it = shaders.find(currentShaderID);
// 	if (it == shaders.end()) {
// 		std::cerr << BRed << "Error: Shader ID " << currentShaderID << " not found." << ResetColor << std::endl;
// 		return 0;
// 	}
// 	if (it == shaders.begin())
// 		it = shaders.end();
// 	--it;

// 	if (VERBOSE)
// 		std::cout << "Now using shader " << it->first << " \"" << it->second.shaderName << "\"" << std::endl;

// 	currentShaderID = it->first;
// 	glUseProgram(currentShaderID);

// 	return currentShaderID;
// }
// /// ---



// /// Uniforms setters

// // Set a boolean uniform
// void Shader::setUniform(const GLuint &shaderID, const std::string &name, bool value) {
// 	glUniform1i(glGetUniformLocation(shaderID, name.c_str()), (int)value);
// }

// // Set an integer uniform
// void Shader::setUniform(const GLuint &shaderID, const std::string &name, int value) {
// 	glUniform1i(glGetUniformLocation(shaderID, name.c_str()), value);
// }

// // Set a float uniform
// void Shader::setUniform(const GLuint &shaderID, const std::string &name, float value) {
// 	glUniform1f(glGetUniformLocation(shaderID, name.c_str()), value);
// }

// // Set a vec2 uniform
// void Shader::setUniform(const GLuint &shaderID, const std::string &name, glm::vec2 value) {
// 	glUniform2f(glGetUniformLocation(shaderID, name.c_str()), value[0], value[1]);
// }

// // Set a vec3 uniform
// void Shader::setUniform(const GLuint &shaderID, const std::string &name, glm::vec3 value) {
// 	glUniform3f(glGetUniformLocation(shaderID, name.c_str()), value[0], value[1], value[2]);
// }

// // Set a vec4 uniform
// void Shader::setUniform(const GLuint &shaderID, const std::string &name, glm::vec4 value) {
// 	glUniform4f(glGetUniformLocation(shaderID, name.c_str()), value[0], value[1], value[2], value[3]);
// }

// // Set a mat4 uniform
// void Shader::setUniform(const GLuint &shaderID, const std::string &name, glm::mat4 value) {
// 	glUniformMatrix4fv(glGetUniformLocation(shaderID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
// }
// /// ---



// /// Getters

// // Return the id of the current shader
// // By default, the first shader added is the current shader
// const GLuint &Shader::getCurrentShaderID() const {
// 	return currentShaderID;
// }
// /// ---
