#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "util/resource_pool.hpp"
#include "shader.hpp"
#include "util/image.hpp"

using namespace glm;

namespace engine{
    struct Material{
        Material(Handle<Shader> shader, const vec4 baseColor, Image textureImage);

        ~Material();

        Material(const Material&) = delete;
        Material& operator=(const Material&) = delete;
        Material(Material&& other) noexcept;
        Material& operator=(Material&& other) noexcept;

        Handle<Shader> shader{};
        vec4 baseColor{1.0f};
        Image textureImage{};

        GLuint materialGLId = 0;
        GLuint textureGLId = 0;

        void bind() const;
    };

    //C:/msys64/ucrt64/bin/../lib/gcc/x86_64-w64-mingw32/15.2.0/../../../../x86_64-w64-mingw32/bin/ld.exe: build/obj/components/renderer.o:renderer.cpp:(.text+0xed70): multiple definition of `stbi_failure_reason'; build/obj/components/game_object.o:game_object.cp:(.text+0xdd90): first defined here
}