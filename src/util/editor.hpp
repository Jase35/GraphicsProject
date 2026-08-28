#pragma once

#include <GLFW/glfw3.h>
#include <entt/entt.hpp>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "core/ecs_manager.hpp"
#include "core/window.hpp"
#include "components/game_object.hpp"

namespace engine{
    namespace editor{
        void initialize(Window& window);
        void update(Scene& scene);
        void shutdown();

        void renderEntity(Scene& scene, entt::entity entity, const bool& active, const bool& insideDragged);

        inline entt::entity selectedEntity = entt::null;
        inline entt::entity draggedEntity = entt::null;
        inline ImGuiStyle baseStyle{};
    }
}