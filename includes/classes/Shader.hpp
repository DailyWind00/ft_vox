#pragma once

/// Defines
# define COLOR_HEADER_CXX

/// System includes
# include <string>
# include <vector>
# include <map>

/// Dependencies
# include <glad/glad.h>
# include <glm/gtc/type_ptr.hpp>
# include <glm/glm.hpp>

/// Global variables
extern bool VERBOSE;

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
        void    setUniform(const GLuint &shaderID, const std::string &name, glm::vec2 value);
        void    setUniform(const GLuint &shaderID, const std::string &name, glm::vec3 value);
        void    setUniform(const GLuint &shaderID, const std::string &name, glm::vec4 value);
        void    setUniform(const GLuint &shaderID, const std::string &name, glm::mat4 value);

        /// Getters

        const GLuint &getCurrentShaderID() const;
};
