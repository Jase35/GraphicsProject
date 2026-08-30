#pragma once

#include <cstdint>
#include <vector>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "composites/vertex.hpp"

using namespace glm;

namespace engine{
    struct Mesh{
        Mesh(
            const std::vector<Vertex>& vertices,
            const std::vector<uint32_t>& indices
        );
        ~Mesh();
        static Mesh testMesh();
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        GLuint vao, vbo, ebo;
        void bind() const;
    };
}