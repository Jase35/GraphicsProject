#pragma once

#include <iostream>
#include <deque>
#include <filesystem>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <entt/entt.hpp>

#include "ecs_manager.hpp"
#include "window.hpp"
#include "util/editor.hpp"
#include "components/game_object.hpp"
#include "components/camera.hpp"
#include "util/debug.hpp"
#include "util/asset_loading.hpp"
#include "components/renderer.hpp"

namespace engine{
    class App{
        public:
        static constexpr int DEFAULT_WIDTH = 1280;
        static constexpr int DEFAULT_HEIGHT = 720;
        static constexpr float MAX_FRAME_TIME = 0.5f;
        App(std::string appName, const bool devMode);
        int run();
        void update(float deltaTime);
        void loadScene();

        private:
        std::string appName;
        bool devMode;
        Scene scene;
        Resources resources;
        //entt::registry registry;
        //entt::entity firstRoot = entt::null; // First among equals (in depth).
        Window window{"Starting...", DEFAULT_WIDTH, DEFAULT_HEIGHT};
    };
}