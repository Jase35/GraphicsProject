#include "input.hpp"

namespace engine{
    namespace input{
        void update(Scene& scene, Window& window, const float& deltaTime){
            for(auto [entity, camera, gameObject] : scene.registry.view<Camera, GameObject>().each()){
                if(gameObject.active){
                    updateCamera(gameObject, window, deltaTime);
                    return;
                }
            }
        }

        void updateCamera(GameObject& gameObject, Window& window, const float& deltaTime){
            const float SPEED = 3.0f;

            vec3 movement = vec3(0.0f);
            mat4 mat = gameObject.globalTransform;

            vec3 right = normalize(vec3(mat[0]));
            vec3 up = normalize(vec3(mat[1]));
            vec3 forward = normalize(vec3(mat[2]));

            if(glfwGetKey(window.glfwWindow, GLFW_KEY_W)){
                movement -= forward;
            }
            if(glfwGetKey(window.glfwWindow, GLFW_KEY_S)){
                movement += forward;
            }
            if(glfwGetKey(window.glfwWindow, GLFW_KEY_A)){
                movement -= right;
            }
            if(glfwGetKey(window.glfwWindow, GLFW_KEY_D)){
                movement += right;
            }
            if(glfwGetKey(window.glfwWindow, GLFW_KEY_E)){
                movement += up;
            }
            if(glfwGetKey(window.glfwWindow, GLFW_KEY_Q)){
                movement -= up;
            }

            vec3 rotation = vec3(0.0f);
            if(glfwGetKey(window.glfwWindow, GLFW_KEY_RIGHT)){
                rotation += vec3(0.0f, -1.0f, 0.0f);
            }
            if(glfwGetKey(window.glfwWindow, GLFW_KEY_LEFT)){
                rotation += vec3(0.0f, 1.0f, 0.0f);
            }
            if(glfwGetKey(window.glfwWindow, GLFW_KEY_UP)){
                rotation += vec3(1.0f, 0.0f, 0.0f);
            }
            if(glfwGetKey(window.glfwWindow, GLFW_KEY_DOWN)){
                rotation += vec3(-1.0f, 0.0f, 0.0f);
            }
            gameObject.rotation += rotation * deltaTime;

            if(length(movement) > 0.00001f){
                movement = normalize(movement) * SPEED * deltaTime;

                gameObject.position += movement;

                //vec3 pos = gameObject.position;
                //printf((std::to_string(pos.x) + ", " + std::to_string(pos.y) + ", " + std::to_string(pos.z) + "\n").c_str());
            }
        }
    }
}