#include "window.hpp"

namespace engine{
    Window::Window(std::string title, int width, int height) : title(title), width(width), height(height) {
        if(!glfwInit()){
            throw std::runtime_error("Failed to initialize GLFW.");
        }
        
        glfwWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if(!glfwWindow){
            throw std::runtime_error("Failed to initialize window.");
        }
        glfwMakeContextCurrent(glfwWindow);
    }

    Window::~Window(){
        glfwDestroyWindow(glfwWindow);
    }

    bool Window::isRunning(){
        return !glfwWindowShouldClose(glfwWindow);
    }

    void Window::setTitle(std::string newTitle){
        title = newTitle;
        glfwSetWindowTitle(glfwWindow, title.c_str());
    }

    void Window::swapBuffers(){
        glfwSwapBuffers(glfwWindow);
    }

    float Window::getAspectRatio() const {
        int actualWidth, actualHeight;
        glfwGetWindowSize(glfwWindow, &actualWidth, &actualHeight);
        return ((float)actualWidth) / ((float)actualHeight);
    }

    void Window::check(){
        int actualWidth, actualHeight;
        glfwGetWindowSize(glfwWindow, &actualWidth, &actualHeight);
        glViewport(0, 0, actualWidth, actualHeight);
    }
}