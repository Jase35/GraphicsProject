#include "app.hpp"

namespace engine{
    App::App(std::string appName, const bool devMode) : appName(appName), devMode(devMode) {}

    int App::run(){
        if(!gladLoadGL((GLADloadfunc)glfwGetProcAddress)){
            printf("Failed to initialize GLAD.\n");
            return -1;
        }

        glEnable(GL_DEPTH_TEST);

        if(devMode){
            editor::initialize(window);
        }

        ecs_manager::initialize();

        loadScene();

        int ticks = 0;
        float tickTimer = 0.0f;
        double lastTime = glfwGetTime();
        window.setTitle(appName);
        while(window.isRunning()){
            glfwPollEvents();

            double currentTime = glfwGetTime();
            float deltaTime = static_cast<float>(currentTime - lastTime);
            if(deltaTime > MAX_FRAME_TIME){
                // Prevent spiraling.
                deltaTime = MAX_FRAME_TIME;
            }
            lastTime = currentTime;

            window.check();

            glClearColor(0.2f, 0.0f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            update(deltaTime);

            if(devMode){
                editor::update(scene);
                ticks++;
                tickTimer += deltaTime;
                if(tickTimer >= 1.0f){
                    window.setTitle(appName + " [" + std::to_string(ticks) + " TPS]");
                    ticks = 0;
                    tickTimer = 0.0f;
                }
            }

            window.swapBuffers();
        }

        if(devMode){
            editor::shutdown();
        }

        glfwTerminate();
        return 0;
    }

    void App::loadScene(){
        scene.clear();

        // TODO maybe change to be dynamic. In the mean time, nothing above here should be touched.

        scene.firstRoot = scene.registry.create();
        GameObject& firstGameObject = scene.registry.emplace<GameObject>(scene.firstRoot, "Camera");
        scene.registry.emplace<Camera>(scene.firstRoot);
        firstGameObject.position = vec3(0.0f, 0.0f, 1.0f); // May need to make positive.

        std::filesystem::path path = std::filesystem::current_path() / "res" / "models" / "miku.glb";
        entt::entity secondEntity = asset_loading::importRootEntity_fromWholeGLB(scene, path);
        game_object_system::setParent(scene, secondEntity, entt::null);
        GameObject& secondGameObject = scene.registry.get<GameObject>(secondEntity);
        secondGameObject.scale = vec3(0.01f);
        secondGameObject.position = vec3(0.0f, 0.0f, -5.0f);
        secondGameObject.rotation.x = 1.57f;

        entt::entity thirdEntity = asset_loading::importRootEntity_fromWholeGLB(scene, path);
        game_object_system::setParent(scene, thirdEntity, secondEntity);
        GameObject& thirdGameObject = scene.registry.get<GameObject>(thirdEntity);
        thirdGameObject.scale = vec3(0.5f);
        thirdGameObject.position = vec3(1.0f, 0.0f, 0.0f);

        render_system::tempShader = render_system::loadShader(
            std::filesystem::current_path() / "res" / "shaders" / "basic.vert",
            std::filesystem::current_path() / "res" / "shaders" / "basic.frag"
        );
    }

    void App::update(float deltaTime){
        game_object_system::debugUpdate(scene.registry, deltaTime);
        game_object_system::prerender(scene, scene.firstRoot, mat4(1.0f)); // Pass in identity.

        render_system::render(scene.registry, window.getAspectRatio());
    }
}