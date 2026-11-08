#include "window.hpp"

namespace engine{
    Window::Window(std::string title, int width, int height) : title(title), width(width), height(height) {
        if(!glfwInit()){
            throw std::runtime_error("Failed to initialize GLFW.");
        }

        glfwWindowHint(GLFW_SAMPLES, MSAA_SAMPLES);
        
        glfwWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if(!glfwWindow){
            throw std::runtime_error("Failed to initialize window.");
        }
        glfwMakeContextCurrent(glfwWindow);
        glfwSwapInterval(0);
    }

    Window::~Window(){
        glfwDestroyWindow(glfwWindow);
        glfwTerminate();
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
        ivec2 res = getResolution();
        return ((float)res.x) / ((float)res.y);
    }

    ivec2 Window::getResolution() const {
        return ivec2(width, height);
    }

    bool Window::checkResized(){
        int actualWidth, actualHeight;
        glfwGetWindowSize(glfwWindow, &actualWidth, &actualHeight);
        bool result = false;
        if(width != actualWidth){
            width = actualWidth;
            result = true;
        }
        if(height != actualHeight){
            height = actualHeight;
            result = true;
        }
        glViewport(0, 0, actualWidth, actualHeight);
        return result;
    }
}