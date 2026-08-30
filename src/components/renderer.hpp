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
#include "composites/shader.hpp"
#include "util/resource_pool.hpp"

using namespace glm;

namespace engine{
    struct Scene;
    struct InstanceData{
        mat4 modelMatrix;
        mat4 normalMatrix;
    };
    
    inline constexpr size_t MAX_BONES = 128;
    
    struct Renderer{
        Renderer(Handle<Mesh> mesh, Handle<Shader> shader);
        ~Renderer();
        uint8_t layer = 127;
        Handle<Mesh> mesh;
        Handle<Shader> shader;
    };

    namespace render_system{
        void render(Scene& scene, ResourcePool<Mesh>& meshes, ResourcePool<Shader>& shaders, const float& aspectRatio);
    }
}