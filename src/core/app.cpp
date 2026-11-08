#include "app.hpp"

namespace engine{
    App::App(std::string appName, const bool editorOpen, const bool titleDebug, const bool oneFrame) : appName(appName), editorOpen(editorOpen), titleDebug(titleDebug), oneFrame(oneFrame) {}

    int App::run(){
        if(!gladLoadGL((GLADloadfunc)glfwGetProcAddress)){
            printf("Failed to initialize GLAD.\n");
            return -1;
        }

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        glEnable(GL_MULTISAMPLE);

        if(editorOpen){
            editor::initialize(window);
        }

        ecs_manager::initialize();

        loadScene();

        runFrames();

        if(editorOpen){
            editor::shutdown();
        }

        render_system::shutdown();
        scene.clear();

        return 0;
    }

    void App::runFrames(){
        int ticks = 0;
        float tickTimer = 0.0f;
        double lastTime = glfwGetTime();
        window.setTitle(appName);
        bool firstFrame = true;
        while(window.isRunning()){
            glfwPollEvents();
            bool resized = window.checkResized();

            if(!oneFrame || (firstFrame || resized)){
                float deltaTime = 0.0f;
                if(!oneFrame){
                    double currentTime = glfwGetTime();
                    deltaTime = static_cast<float>(currentTime - lastTime);
                    if(deltaTime > MAX_FRAME_TIME){
                        // Prevent spiraling.
                        deltaTime = MAX_FRAME_TIME;
                    }
                    lastTime = currentTime;
                }

                //window.check();

                input::update(scene, window, deltaTime);

                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                update(deltaTime);

                if(editorOpen){
                    editor::update(scene);
                }

                if(titleDebug){
                    ticks++;
                    tickTimer += deltaTime;
                    if(tickTimer >= 1.0f){
                        window.setTitle(appName + " [" + std::to_string(ticks) + " TPS]");
                        ticks = 0;
                        tickTimer = 0.0f;
                    }
                }

                window.swapBuffers();

                firstFrame = false;
            }
        }
    }

    void App::loadScene(){
        scene = Scene{};

        // TODO maybe change to be dynamic. In the mean time, nothing above here should be touched.

        /*Handle<Mesh> mesh = scene.resources.meshes.add(asset_loading::importMesh_fromFirstGLB(
            std::filesystem::current_path() / "res" / "models" / "miku.glb"
        ));*/

        Handle<Shader> shader = scene.resources.shaders.add(Shader(
            std::filesystem::current_path() / "res" / "shaders" / "mesh.vert",
            std::filesystem::current_path() / "res" / "shaders" / "mesh.frag"
        ));

        //Handle<Material> material = scene.resources.materials.add(Material(shader, vec4(1.0f, 1.0f, 1.0f, 1.0f), true, 0));

        Handle<Prefab> prefab = scene.resources.prefabs.add(asset_loading::importRootEntity_fromGLB(
            std::filesystem::current_path() / "res" / "models" / "miku.glb",
            scene.resources,
            shader
        ));

        scene.firstRoot = scene.registry.create();
        GameObject& firstGameObject = scene.registry.emplace<GameObject>(scene.firstRoot, "Camera");
        scene.registry.emplace<Camera>(scene.firstRoot);
        firstGameObject.position = vec3(0.0f, 0.0f, 1.0f); // May need to make positive.

        //entt::entity secondEntity = asset_loading::importRootEntity_fromWholeGLB(scene, path);
        /*entt::entity secondEntity = scene.registry.create();
        GameObject& secondGameObject = scene.registry.emplace<GameObject>(secondEntity, "Second");
        scene.registry.emplace<Renderer>(secondEntity, mesh, material);
        game_object_system::setParent(scene, secondEntity, entt::null);
        secondGameObject.scale = vec3(0.01f);
        secondGameObject.position = vec3(0.0f, 0.0f, -5.0f);
        secondGameObject.rotation.x = 1.57f;*/

        Handle<Shader> sphereShader = scene.resources.shaders.add(
            Shader(
                std::filesystem::current_path() / "res" / "shaders" / "sphere.vert",
                std::filesystem::current_path() / "res" / "shaders" / "sphere.frag"
            )
        );

        {
            entt::entity secondEntity = scene.registry.create();
            GameObject& secondGameObject = scene.registry.emplace<GameObject>(secondEntity, "Sphere");
            scene.registry.emplace<SphereRenderer>(secondEntity, scene.resources.materials.add(
                Material(
                    sphereShader,
                    vec4{1.0f, 0.0f, 0.5f, 1.0f},
                    Image{}
                )
            ));
            game_object_system::setParent(scene, secondEntity, entt::null);
            secondGameObject.position = vec3(2.0f, 1.0f, -10.0f);
        }

        {
            entt::entity secondEntity = scene.registry.create();
            GameObject& secondGameObject = scene.registry.emplace<GameObject>(secondEntity, "Sphere");
            scene.registry.emplace<SphereRenderer>(secondEntity, scene.resources.materials.add(
                Material(
                    sphereShader,
                    vec4{0.0f, 1.0f, 0.5f, 1.0f},
                    Image{}
                )
            ));
            game_object_system::setParent(scene, secondEntity, entt::null);
            secondGameObject.position = vec3(-2.0f, 1.0f, -15.0f);
        }

        {
            entt::entity secondEntity = scene.registry.create();
            GameObject& secondGameObject = scene.registry.emplace<GameObject>(secondEntity, "Sphere");
            scene.registry.emplace<SphereRenderer>(secondEntity, scene.resources.materials.add(
                Material(
                    sphereShader,
                    vec4{0.5f, 1.0f, 0.0f, 1.0f},
                    Image{}
                )
            ));
            game_object_system::setParent(scene, secondEntity, entt::null);
            secondGameObject.position = vec3(0.0f, 2.0f, -12.0f);
        }

        size_t ITERATIONS = 1;
        Prefab* prefabResource = scene.resources.prefabs.get(prefab);
        for(size_t i = 0; i < ITERATIONS; i++){
            /*entt::entity newEntity = scene.registry.create();
            GameObject& newGameObject = scene.registry.emplace<GameObject>(newEntity, "New");
            scene.registry.emplace<Renderer>(newEntity, mesh, material, shader);
            game_object_system::setParent(scene, newEntity, entt::null);
            newGameObject.scale = vec3(0.0075f);
            newGameObject.position = vec3(0.0f, 0.0f, -5.0f);
            newGameObject.rotation.x = (((float)i) / ITERATIONS) * 6.28f;*/
            entt::entity newEntity = scene.instantiate(*prefabResource);
            GameObject& gameObject = scene.registry.get<GameObject>(newEntity);
            gameObject.scale = vec3(0.0075f);
            //gameObject.scale = vec3(1.0f);
            gameObject.position = vec3(0.0f, 0.0f, -5.0f);
            gameObject.rotation.z = 1.57f;
            gameObject.rotation.x = (((float)i) / ITERATIONS) * 6.28f;
        }

        //entt::entity thirdEntity = asset_loading::importRootEntity_fromWholeGLB(scene, path);
        /*entt::entity thirdEntity = scene.registry.create();
        GameObject& thirdGameObject = scene.registry.emplace<GameObject>(thirdEntity, "Third");
        scene.registry.emplace<Renderer>(thirdEntity, mesh, material);
        game_object_system::setParent(scene, thirdEntity, secondEntity);
        thirdGameObject.scale = vec3(0.5f);
        thirdGameObject.position = vec3(1.0f, 0.0f, 0.0f);*/
    }

    void App::update(float deltaTime){
        game_object_system::debugUpdate(scene.registry, deltaTime);
        game_object_system::prerender(scene);

        render_system::render(scene, window.getResolution(), window.getAspectRatio());
    }
}