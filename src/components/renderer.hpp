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
        vec4 normalMatrix[3];
        vec4 padding;
    };
    
    inline constexpr GLint INSTANCE_OFFSET_LOCATION = 0;
    inline constexpr size_t MAX_BONES = 128;
    
    struct Renderer{
        Renderer(Handle<Mesh> mesh, Handle<Material> material);
        uint8_t layer = 127;
        Handle<Mesh> mesh;
        Handle<Material> material;
    };

    namespace render_system{
        void render(Scene& scene, const float& aspectRatio);
        void shutdown();
    }
}