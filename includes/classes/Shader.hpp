#pragma once

/// Defines
# define COLOR_HEADER_CXX

/// System includes
# include <iostream>
# include <algorithm>
# include <vector>
# include <array>
# include <map>
# include <sstream>
# include <fstream>

/// Dependencies
# include <glad/glad.h>
# include "color.h"

/// Global variables
extern bool VERBOSE;

typedef std::array<float, 2> vec2;
typedef std::array<float, 3> vec3;
typedef std::array<float, 4> vec4;
typedef std::array<float, 16> mat4;

typedef struct shaderData {
    GLuint		shaderID;
    std::string	vertexPath;
    std::string	fragmentpath;
    std::string	shaderName;
}   shaderData;

typedef     std::map<GLuint, shaderData>::iterator ShaderIterator;
typedef     std::pair<GLuint, shaderData> ShaderPair;

class Shader {
    private:
        std::map<GLuint, shaderData>	shaders;
        std::vector<GLuint>				shaderIDs;
        GLuint							currentShaderID;

        GLuint  make_module(const std::string &filepath, GLuint module_type);
        GLuint  make_shader(const std::string &vertex_path, const std::string &fragment_path);

    public:
        Shader();
        ~Shader();


        /// public functions

        void    use(GLuint shaderID);
        GLuint  recompile(GLuint shaderID);
        GLuint  add_shader(const std::string &vertexPath, const std::string &fragmentPath, const std::string &shaderName);
        void    remove_shader(GLuint shaderID);
        GLuint	SetNextShader();
        GLuint	SetPreviousShader();


        /// Uniforms setters

        void    setUniform(const GLuint &shaderID, const std::string &name, bool value);
        void    setUniform(const GLuint &shaderID, const std::string &name, int value);
        void    setUniform(const GLuint &shaderID, const std::string &name, float value);
        void    setUniform(const GLuint &shaderID, const std::string &name, vec2 value);
        void    setUniform(const GLuint &shaderID, const std::string &name, vec3 value);
        void    setUniform(const GLuint &shaderID, const std::string &name, vec4 value);
        void    setUniform(const GLuint &shaderID, const std::string &name, mat4 value);

        /// Getters

        const GLuint &getCurrentShaderID() const;
};