#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "components/game_object.hpp"
#include "components/renderer.hpp"
#include "composites/mesh.hpp"
#include "composites/shader.hpp"
#include "util/resource_pool.hpp"

namespace engine{
    struct RenderItem{
        uint64_t key;
        // 63..56 layer (256)
        // 55..40 shader (65536)
        // 39..24 material (65536)
        // 23..8 mesh (65536)
        // 7..0 miscellaneous (256)
        entt::entity entity;
        mat4 transform;
    };

    struct Scene{
        entt::registry registry;
        entt::entity firstRoot = entt::null;
        std::vector<RenderItem> renderList;

        void clear(){
            registry.clear();
            firstRoot = entt::null;
            renderList.clear();
            // TODO invalidate asset batching import cache.
        }
    };

    struct Resources{
        ResourcePool<Mesh> meshes;
        ResourcePool<Shader> shaders;

        void clear(){
            meshes.clear();
            shaders.clear();
        }
    };

    namespace ecs_manager{
        void initialize();
        void initializeComponents();
    }
}