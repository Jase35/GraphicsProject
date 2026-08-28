#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <filesystem>
#include <entt/entt.hpp>
#include <cmath>
#include <cstddef>

#include "components/game_object.hpp"
#include "components/camera.hpp"
#include "composites/mesh.hpp"

using namespace glm;

namespace engine{
    struct Renderer{
        Renderer(const Mesh& mesh);
        Renderer();
        ~Renderer();
        Mesh mesh;
        GLuint vao, vbo, ebo;
    };

    namespace render_system{
        inline GLuint tempShader;
        GLuint loadShader(
            const std::filesystem::path& vertPath,
            const std::filesystem::path& fragPath
        );
        void render(entt::registry& registry, const float& aspectRatio);
        std::string readFile(const std::filesystem::path& path);
    }
}