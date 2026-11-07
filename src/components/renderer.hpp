#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <entt/entt.hpp>
#include <array>
#include <cmath>
#include <cstddef>
#include <algorithm>
#include <utility>
#include <vector>

#include "components/game_object.hpp"
#include "components/camera.hpp"
#include "composites/mesh.hpp"
#include "composites/material.hpp"
#include "util/resource_pool.hpp"

using namespace glm;

namespace engine{
    struct Scene;
    struct InstanceData{
        mat4 modelMatrix;
        mat4 inverseModelMatrix;
        vec4 normalMatrix[3];
        uvec4 geometry;
        vec4 boundMin;
        vec4 boundMax;
    };

    struct GPUVertex{
        vec4 position;
        vec4 normal;
    };

    struct GPUSphere{
        vec4 baseColor;
        vec3 position;
        float diameter;
    };
    
    inline constexpr GLint INSTANCE_OFFSET_LOCATION = 0;
    inline constexpr size_t MAX_BONES = 128;
    
    struct Renderer{
        Renderer(Handle<Mesh> mesh, Handle<Material> material);
        uint8_t layer = 127;
        Handle<Mesh> mesh;
        Handle<Material> material;
    };

    struct SphereRenderer{
        SphereRenderer(Handle<Material> material);
        uint8_t layer = 127;
        Handle<Material> material;
    };

    namespace render_system{
        void render(Scene& scene, const ivec2& resolution, const float& aspectRatio);
        void shutdown();
    }
}