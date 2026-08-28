#pragma once

#include <glm/glm.hpp>

using namespace glm;

namespace engine{
    struct Vertex{
        vec3 position{0.0f};
        vec3 normal{0.0f};
        vec2 textureUV{0.0f};
        ivec4 boneIndices{0};
        vec4 boneWeights{0.0f};

        Vertex() = default;

        Vertex(
            const vec3& position
        ) : position(position) {}

        Vertex(
            const vec3& position,
            const vec3& normal,
            const vec2& textureUV
        ) : position(position), normal(normal), textureUV(textureUV) {}

        Vertex(
            const vec3& position,
            const vec3& normal,
            const vec2& textureUV,
            const ivec4& boneIndices,
            const vec4& boneWeights
        ) : position(position), normal(normal), textureUV(textureUV), boneIndices(boneIndices), boneWeights(boneWeights) {}
    };
}