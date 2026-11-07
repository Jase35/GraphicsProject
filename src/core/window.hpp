#pragma once

#include <cstdio>
#include <stdexcept>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

using namespace glm;

namespace engine{
    class Window{
        public:
        Window(std::string title, int width, int height);
        ~Window();
        bool isRunning();
        void setTitle(std::string newTitle);
        void swapBuffers();
        float getAspectRatio() const;
        ivec2 getResolution() const;
        GLFWwindow* glfwWindow;
        bool checkResized();
        
        private:
        std::string title;
        int width = 800;
        int height = 600;
    };
}