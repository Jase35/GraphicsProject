#pragma once

#include <cstdio>
#include <stdexcept>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace engine{
    class Window{
        public:
        Window(std::string title, int width, int height);
        ~Window();
        bool isRunning();
        void setTitle(std::string newTitle);
        void swapBuffers();
        float getAspectRatio() const;
        GLFWwindow* glfwWindow;
        void check();
        
        private:
        std::string title;
        int width = 800;
        int height = 600;
    };
}