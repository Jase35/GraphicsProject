#pragma once

#include <glm/glm.hpp>

#include "ecs_manager.hpp"
#include "window.hpp"

using namespace glm;

namespace engine{
    namespace input{
        void update(Scene& scene, Window& window, const float& deltaTime);
        void updateCamera(GameObject& gameObject, Window& window, const float& deltaTime);
    }
}