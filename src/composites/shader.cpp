#include "shader.hpp"

namespace engine{
    Shader::Shader(
        const std::filesystem::path& vertPath,
        const std::filesystem::path& fragPath
    ){
        std::string vertexShaderSource = readFile(vertPath);
        std::string fragmentShaderSource = readFile(fragPath);

        const GLchar* vertexShaderCode = vertexShaderSource.c_str();
        const GLchar* fragmentShaderCode = fragmentShaderSource.c_str();
        
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderCode, NULL);
        glCompileShader(vertexShader);

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
        glCompileShader(fragmentShader);

        shaderId = glCreateProgram();
        glAttachShader(shaderId, vertexShader);
        glAttachShader(shaderId, fragmentShader);
        glLinkProgram(shaderId);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    Shader::~Shader(){
        // TODO
    }

    void Shader::bind() const{
        glUseProgram(shaderId);
    }

    std::string Shader::readFile(const std::filesystem::path& path){
        std::ifstream f(path.string(), std::ios::in | std::ios::binary);
        if(!f) throw std::runtime_error("Failed to open " + path.string());
        std::ostringstream ss;
        ss << f.rdbuf();
        return ss.str();
    }
}