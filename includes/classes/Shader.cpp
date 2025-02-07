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
	std::vector<GLuint> shadersIDs;

	shadersIDs.push_back(make_module(vertexPath, GL_VERTEX_SHADER));
	shadersIDs.push_back(make_module(fragmentPath, GL_FRAGMENT_SHADER));
	if (!geometryPath.empty())
		shadersIDs.push_back(make_module(geometryPath, GL_GEOMETRY_SHADER));

	if (VERBOSE)
		std::cout << "> Linking shader -> ";

	GLuint shader = glCreateProgram();
	for (GLuint module = 0; module < shadersIDs.size(); module++) {
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





/// Constructors & Destructors 
ShaderHandler::ShaderHandler() {
}

ShaderHandler::~ShaderHandler() {
	remove_all_shaders();
}
/// ---



/// Public functions

// Use the shader given with the shaderID,
void ShaderHandler::use(GLuint shaderID) {
	if (currentShaderID == shaderID)
		return;

	for (Shader *shader : shaders) {
		if (shader->getID() == shaderID) {
			shader->use();
			currentShaderID = shaderID;
			return;
		}
	}
}

// Use the shader given with the shader iterator
void ShaderHandler::use(VShaders::const_iterator shader) {
	if (currentShaderID == (*shader)->getID())
		return;

	(*shader)->use();
	currentShaderID = (*shader)->getID();
}

// Recompile the shader given with the shaderID
// Return the id of the new recompiled shader
// If the shader is not found, return the current shader ID
GLuint	ShaderHandler::recompile(GLuint shaderID) {
	for (Shader *shader : shaders) {
		if (shader->getID() == shaderID) {
			shader->recompile();
			if (shaderID == currentShaderID)
				currentShaderID = shader->getID();
				
			return shader->getID();
		}
	}

	return currentShaderID;
}

// Add a new shader to the Shaders class, return the id of the new shader
// By default, the first shader added is the current shader
GLuint	ShaderHandler::add_shader(const std::string &vertexPath, const std::string &fragmentPath) {
	shaders.push_back( new Shader(vertexPath, fragmentPath) );

	if (shaders.size() == 1)
		currentShaderID = shaders[0]->getID();

	return shaders.back()->getID();
}

// Add a new shader to the Shaders class, return the id of the new shader
// By default, the first shader added is the current shader
GLuint ShaderHandler::add_shader(const std::string &vertexPath, const std::string &fragmentPath, const std::string &geometryPath) {
	shaders.push_back( new Shader(vertexPath, fragmentPath, geometryPath) );

	if (shaders.size() == 1)
		currentShaderID = shaders[0]->getID();

	return shaders.back()->getID();
}

// Remove a shader from the Shaders class
void	ShaderHandler::remove_shader(GLuint shaderID) {
	for (VShaders::iterator it = shaders.begin(); it != shaders.end(); it++) {
		if ((*it)->getID() == shaderID) {
			delete *it;
			shaders.erase(it);
			return;
		}
	}
}

// Remove all shaders from the Shaders class
void	ShaderHandler::remove_all_shaders() {
	for (Shader *shader : shaders)
		delete shader;
	shaders.clear();
}

// Set the next shader of the current shader as the current shader
// Return the id of the new current shader
GLuint	ShaderHandler::setNextShader() {
	for (VShaders::const_iterator it = shaders.begin(); it != shaders.end(); it++) {
		if ((*it)->getID() == currentShaderID) {

			if (it == back()) { // Loop back to the first shader
				use(shaders.front()->getID());
				return shaders.front()->getID();
			}

			use((*(it + 1))->getID());
			return (*(it + 1))->getID();
		}
	}

	return currentShaderID;
}

// Set the previous shader of the current shader as the current shader
// Return the id of the new current shader
GLuint	ShaderHandler::setPreviousShader() {
	for (VShaders::const_iterator it = shaders.begin(); it != shaders.end(); it++) {
		if ((*it)->getID() == currentShaderID) {

			if (it == begin()) { // Loop back to the last shader
				use(shaders.back()->getID());
				return shaders.back()->getID();
			}

			use((*(it - 1))->getID());
			return (*(it - 1))->getID();
		}
	}

	return currentShaderID;
}
/// ---



/// Uniforms setters

// Set a boolean uniform
void ShaderHandler::setUniform(const GLuint &shaderID, const std::string &name, bool value) {
	glUseProgram(shaderID);
	glUniform1i(glGetUniformLocation(shaderID, name.c_str()), (int)value);
	glUseProgram(currentShaderID);
}

// Set an integer uniform
void ShaderHandler::setUniform(const GLuint &shaderID, const std::string &name, int value) {
	glUseProgram(shaderID);
	glUniform1i(glGetUniformLocation(shaderID, name.c_str()), value);
	glUseProgram(currentShaderID);
}

// Set a float uniform
void ShaderHandler::setUniform(const GLuint &shaderID, const std::string &name, float value) {
	glUseProgram(shaderID);
	glUniform1f(glGetUniformLocation(shaderID, name.c_str()), value);
	glUseProgram(currentShaderID);
}

// Set a vec2 uniform
void ShaderHandler::setUniform(const GLuint &shaderID, const std::string &name, glm::vec2 value) {
	glUseProgram(shaderID);
	glUniform2f(glGetUniformLocation(shaderID, name.c_str()), value[0], value[1]);
	glUseProgram(currentShaderID);
}

// Set a vec3 uniform
void ShaderHandler::setUniform(const GLuint &shaderID, const std::string &name, glm::vec3 value) {
	glUseProgram(shaderID);
	glUniform3f(glGetUniformLocation(shaderID, name.c_str()), value[0], value[1], value[2]);
	glUseProgram(currentShaderID);
}

// Set a vec4 uniform
void ShaderHandler::setUniform(const GLuint &shaderID, const std::string &name, glm::vec4 value) {
	glUseProgram(shaderID);
	glUniform4f(glGetUniformLocation(shaderID, name.c_str()), value[0], value[1], value[2], value[3]);
	glUseProgram(currentShaderID);
}

// Set a mat4 uniform
void ShaderHandler::setUniform(const GLuint &shaderID, const std::string &name, glm::mat4 value) {
	glUseProgram(shaderID);
	glUniformMatrix4fv(glGetUniformLocation(shaderID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
	glUseProgram(currentShaderID);
}
/// ---



/// Getters

// Return the id of the current shader
const GLuint &ShaderHandler::getCurrentShaderID() const {
	return currentShaderID;
}

// Return the current shader
const Shader *ShaderHandler::getCurrentShader() const {
	for (Shader *shader : shaders)
		if (shader->getID() == currentShaderID)
			return shader;
	
	return nullptr;
}

// Return the shader with the given id
// If the shader is not found, return the current shader
const Shader *ShaderHandler::getShader(const GLuint &shaderID) const {
	for (Shader *shader : shaders)
		if (shader->getID() == shaderID)
			return shader;

	return getCurrentShader();
}

// Return the shader at the given index in the vector
VShaders::const_iterator	ShaderHandler::operator[](const size_t &index) const {
	if (index >= shaders.size())
		return shaders.cend();
	return shaders.cbegin() + index;
}

// Return the begin of the shaders vector
VShaders::const_iterator	ShaderHandler::begin() const {
	return shaders.cbegin();
}

// Return the first shader in the vector
VShaders::const_iterator	ShaderHandler::front() const {
	return shaders.cbegin();
}

// Return the last shader in the vector
VShaders::const_iterator	ShaderHandler::back() const {
	return shaders.cend() - 1;
}

// Return the end of the shaders vector
VShaders::const_iterator	ShaderHandler::end() const {
	return shaders.cend();
}
/// ---
