#pragma once

/// Defines
# define COLOR_HEADER_CXX

/// System includes
# include <iostream>
# include <algorithm>
# include <sstream>
# include <fstream>
# include <vector>

/// Dependencies
# include <glad/glad.h>
# include <glm/gtc/type_ptr.hpp>
# include <glm/glm.hpp>
# include "color.h"

/// Global variables
extern bool VERBOSE;

// Single shader class
// Handle the creation, destruction and usage of a shader
// You need to initialize glad before creating a shader (the Window class does it)
class Shader {
	friend class ShaderHandler;

	private:
		GLuint		shaderID;
		std::string	vertexPath;
		std::string	fragmentPath;
		std::string	geometryPath;

		/// Private functions

		GLuint	make_module(const std::string &filepath, GLuint module_type);
		GLuint	make_shader();

	public:
		Shader(
			const std::string &vertexPath,
			const std::string &fragmentPath,
			const std::string &geometryPath = ""
		);
		~Shader();

		/// Public functions

		void	use();
		void	recompile();

		/// Uniforms setters

        void    setUniform(const std::string &name, bool value);
        void    setUniform(const std::string &name, int value);
        void    setUniform(const std::string &name, float value);
        void    setUniform(const std::string &name, glm::vec2 value);
        void    setUniform(const std::string &name, glm::vec3 value);
        void    setUniform(const std::string &name, glm::vec4 value);
        void    setUniform(const std::string &name, glm::mat4 value);

		/// Getters

		const GLuint &getID() const;
};

typedef std::vector<Shader *>	VShaders;

// Optionnal multiple shaders handling class
// Handle the creation, destruction and usage of multiple shaders
// You need to initialize glad before creating a shader (the Window class does it)
class ShaderHandler {
    private:
        std::vector<Shader *>	shaders;
        GLuint					currentShaderID;

    public:
        ShaderHandler();
        ~ShaderHandler();

        /// public functions

        void    use(GLuint shaderID);
		void	use(VShaders::const_iterator shader);
        GLuint  recompile(GLuint shaderID);
        GLuint  add_shader(const std::string &vertexPath, const std::string &fragmentPath);
		GLuint	add_shader(const std::string &vertexPath, const std::string &fragmentPath, const std::string &geometryPath);
        void    remove_shader(GLuint shaderID);
		void    remove_all_shaders();
        GLuint	setNextShader();
        GLuint	setPreviousShader();


        /// Uniforms setters

        void    setUniform(const GLuint &shaderID, const std::string &name, bool value);
        void    setUniform(const GLuint &shaderID, const std::string &name, int value);
        void    setUniform(const GLuint &shaderID, const std::string &name, float value);
        void    setUniform(const GLuint &shaderID, const std::string &name, glm::vec2 value);
        void    setUniform(const GLuint &shaderID, const std::string &name, glm::vec3 value);
        void    setUniform(const GLuint &shaderID, const std::string &name, glm::vec4 value);
        void    setUniform(const GLuint &shaderID, const std::string &name, glm::mat4 value);

        /// Getters

        const GLuint 			   &getCurrentShaderID() const;
		const Shader 			   *getCurrentShader() const;
		const Shader 			   *getShader(const GLuint &shaderID) const;
        VShaders::const_iterator    operator[](const size_t &index) const;
        VShaders::const_iterator    begin() const;
        VShaders::const_iterator    front() const;
        VShaders::const_iterator    back() const;
        VShaders::const_iterator    end() const;
};
