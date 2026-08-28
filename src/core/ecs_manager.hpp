#pragma once

#include <entt/entt.hpp>

#include "components/game_object.hpp"

namespace engine{
    struct Scene{
        entt::registry registry;
        entt::entity firstRoot = entt::null;

        void clear(){
            registry.clear();
            firstRoot = entt::null;
        }
    };

    namespace ecs_manager{
        void initialize();
        void initializeComponents();
    }
}