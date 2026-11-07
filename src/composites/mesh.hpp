#pragma once

#include <cstdint>
#include <vector>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <utility>
#include <algorithm>
#include <limits>
#include <string>

#include "composites/vertex.hpp"

using namespace glm;

namespace engine{
    inline constexpr uint32_t BVH_LEAF_SIZE = 4;
    inline constexpr uint32_t BVH_MAX_DEPTH = 24;

    struct BVHNode{
        vec3 boundMin;
        uint leftOrStart;
        vec3 boundMax;
        uint count;
    };

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
        std::vector<BVHNode> nodes;
        GLsizei indexCount = 0;
        
        GLuint vao = 0, vbo = 0, ebo = 0;
        vec3 boundMin{0.0}, boundMax{0.0};
        void bind() const;
        void recalculateBounds();
        void buildBVH();
    };
}