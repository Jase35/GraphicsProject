#pragma once

#include <cstdint>
#include <vector>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <utility>

#include "composites/vertex.hpp"

using namespace glm;

namespace engine{
    struct Mesh{
        Mesh(
            const std::vector<Vertex>& vertices,
            const std::vector<uint32_t>& indices
        );
        ~Mesh();

        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;
        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(Mesh&& other) noexcept;
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        GLsizei indexCount = 0;
        
        GLuint vao = 0, vbo = 0, ebo = 0;
        void bind() const;
    };
}