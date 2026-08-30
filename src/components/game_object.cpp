#include "game_object.hpp"

#include "core/ecs_manager.hpp"

namespace engine{

    GameObject::GameObject(std::string name) : name(name) {}

    namespace game_object_system{
        void destroy(Scene& scene, entt::entity entity){
            setParent(scene, entity, entt::null);
            scene.registry.destroy(entity);
        }

        void setParent(Scene& scene, entt::entity child, entt::entity parent){
            if(child == entt::null || child == parent || child == scene.firstRoot){
                return;
            }
            
            GameObject& childObj = scene.registry.get<GameObject>(child);

            for(entt::entity ancestor = parent; ancestor != entt::null; ancestor = scene.registry.get<GameObject>(ancestor).parent){
                if(ancestor == child){
                    return;
                }
            }

            entt::entity head = childObj.parent != entt::null 
                ? scene.registry.get<GameObject>(childObj.parent).firstChild
                : scene.firstRoot;

            if(head == child){
                scene.registry.get<GameObject>(childObj.parent).firstChild = childObj.nextSibling;
            }else{
                for(entt::entity sibling = head; sibling != entt::null;){
                    GameObject& siblingObj = scene.registry.get<GameObject>(sibling);
                    if(siblingObj.nextSibling == child){
                        siblingObj.nextSibling = childObj.nextSibling;
                        break;
                    }
                    sibling = siblingObj.nextSibling;
                }
            }

            childObj.parent = entt::null;
            childObj.nextSibling = entt::null;

            if(parent != entt::null){
                GameObject& parentObj = scene.registry.get<GameObject>(parent);
                childObj.nextSibling = parentObj.firstChild;
                parentObj.firstChild = child;
                childObj.parent = parent;
            }else{
                GameObject& rootObj = scene.registry.get<GameObject>(scene.firstRoot);
                childObj.nextSibling = rootObj.nextSibling;
                rootObj.nextSibling = child;
            }
        }

        void debugUpdate(entt::registry& registry, const float& deltaTime){
            for(auto [entity, gameObject] : registry.view<GameObject>(entt::exclude<Camera>).each()){
                //printf((std::to_string(gameObject.rotation.y) + "\n").c_str());
                gameObject.rotation.y += 1.0f * deltaTime;
                gameObject.rotation.y = std::fmod(gameObject.rotation.y, 2 * 3.14159265358979323846);
            }
        }

        void prerender(Scene& scene){
            scene.renderList.clear();
            prerender(scene, scene.firstRoot, mat4(1.0f)); // Pass in identity.
        }

        void prerender(Scene& scene, entt::entity entity, const mat4& ancestorTransform){
            while(entity != entt::null){
                GameObject& gameObject = scene.registry.get<GameObject>(entity);

                if(gameObject.active){
                    mat4 transform = ancestorTransform * getLocalTranform(
                        gameObject.position,
                        gameObject.rotation,
                        gameObject.scale
                    );

                    gameObject.globalTransform = transform;

                    if(const Renderer* renderer = scene.registry.try_get<Renderer>(entity)){
                        RenderItem item;
                        item.entity = entity;
                        item.key = (static_cast<uint64_t>(renderer->layer) << 56) |
                            (static_cast<uint64_t>(renderer->shader.index) << 40) |
                            (static_cast<uint64_t>(0) << 24) | // TODO implement material
                            (static_cast<uint64_t>(renderer->mesh.index) << 8) |
                            (static_cast<uint64_t>(0)); // Might be useful later; IDK.
                        item.transform = transform;
                        scene.renderList.push_back(item);
                    }

                    prerender(scene, gameObject.firstChild, transform);
                }

                entity = gameObject.nextSibling;
            }
        }

        mat4 getLocalTranform(const vec3& position, const vec3& rotation, const vec3& scale){
            const float c0 = std::cos(rotation.y);
            const float s0 = std::sin(rotation.y);
            const float c1 = std::cos(rotation.x);
            const float s1 = std::sin(rotation.x);
            const float c2 = std::cos(rotation.z);
            const float s2 = std::sin(rotation.z);
            return mat4(
                vec4(
                    scale.x * (c0 * c2 + s0 * s1 * s2),
                    scale.x * (c1 * s2),
                    scale.x * (c0 * s1 * s2 - c2 * s0),
                    0.0f
                ),
                vec4(
                    scale.y * (c2 * s0 * s1 - c0 * s2),
                    scale.y * (c1 * c2),
                    scale.y * (c0 * c2 * s1 + s0 * s2),
                    0.0f
                ),
                vec4(
                    scale.z * (c1 * s0),
                    scale.z * (-s1),
                    scale.z * (c0 * c1),
                    0.0f
                ),
                vec4(position, 1.0f)
            );
        }
    }
}