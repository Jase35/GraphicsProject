#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace engine{
    struct Shader{
        Shader(
            const std::filesystem::path& vertPath,
            const std::filesystem::path& fragPath
        );

        ~Shader();

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;
        Shader(Shader&& other) noexcept;
        Shader& operator=(Shader&& other) noexcept;

        std::string readFile(const std::filesystem::path& path);

        GLuint shaderGLId = 0;

        void bind() const;
    };
}