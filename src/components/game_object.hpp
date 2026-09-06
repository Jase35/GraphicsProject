#pragma once

#include <entt/entt.hpp>
#include <cmath>
#include <cstdint>

#include <glm/glm.hpp>

#include "components/camera.hpp"

using namespace glm;

namespace engine{
    struct Scene;
    
    struct GameObject{
        std::string name;
        vec3 position = vec3(0.0f);
        vec3 rotation = vec3(0.0f);
        vec3 scale = vec3(1.0f);

        GameObject(std::string name);

        bool active = true;
        entt::entity parent = entt::null;
        entt::entity firstChild = entt::null;
        entt::entity nextSibling = entt::null;
        mat4 globalTransform = mat4(1.0f); // Default to identity.
    };

    namespace game_object_system{
        void destroy(Scene& scene, entt::entity gameObject);
        void setParent(entt::registry& registry, entt::entity firstRoot, entt::entity child, entt::entity parent);
        void setParent(Scene& scene, entt::entity child, entt::entity parent);

        void debugUpdate(entt::registry& registry, const float& deltaTime);
        void prerender(Scene& scene);
        void prerender(Scene& scene, entt::entity entity, const mat4& ancestorTransform);

        mat4 getLocalTranform(const vec3& position, const vec3& rotation, const vec3& scale);
    }
}