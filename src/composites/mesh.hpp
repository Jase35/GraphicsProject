#pragma once

#include <cstdint>
#include <vector>
#include <glm/glm.hpp>

#include "composites/vertex.hpp"

using namespace glm;

namespace engine{
    struct Mesh{
        Mesh();
        Mesh(
            const std::vector<Vertex>& vertices,
            const std::vector<uint32_t>& indices
        );
        ~Mesh();
        static Mesh testMesh();
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
    };
}