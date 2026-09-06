#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <unordered_map>

#include "components/game_object.hpp"
#include "components/renderer.hpp"
#include "composites/mesh.hpp"
#include "composites/shader.hpp"
#include "util/resource_pool.hpp"

namespace engine{
    struct RenderList{
        // 63..56 layer (256)
        // 55..40 shader (65536)
        // 39..24 material (65536)
        // 23..8 mesh (65536)
        // 7..0 miscellaneous (256)
        std::vector<uint64_t> keys;
        std::vector<mat4> transforms;
        std::vector<entt::entity> entities;

        void add(uint64_t key, entt::entity entity, const mat4& transform){
            keys.push_back(key);
            entities.push_back(entity);
            transforms.push_back(transform);
        }

        void clear(){
            keys.clear();
            entities.clear();
            transforms.clear();
        }

        size_t size() const{
            return keys.size();
        }
    };

    struct Prefab{
        entt::registry prototype{};
        entt::entity firstRoot = entt::null;
    };

    struct Resources{
        ResourcePool<Mesh> meshes;
        ResourcePool<Material> materials;
        ResourcePool<Shader> shaders;
        ResourcePool<Prefab> prefabs;

        void clear(){
            meshes.clear();
            materials.clear();
            shaders.clear();
            prefabs.clear();
        }
    };

    struct Scene{
        entt::registry registry{};
        entt::entity firstRoot = entt::null;
        RenderList renderList{};
        Resources resources{};

        entt::entity clone(entt::registry& destination, const entt::registry& source, entt::entity entity){
            entt::entity result = destination.create();
            for(auto [id, storage] : source.storage()){
                if(storage.contains(entity)){
                    if(auto* destinationStorage = destination.storage(id)){
                        destinationStorage->push(result, storage.value(entity));
                    }
                }
            }
            return result;
        }

        entt::entity instantiate(const Prefab& prefab, entt::entity parent){    
            if(prefab.firstRoot == entt::null){
                return entt::null;
            }

            registry.storage<GameObject>();
            registry.storage<Renderer>();
            
            std::unordered_map<entt::entity, entt::entity> remap;

            for(auto prototype : prefab.prototype.view<entt::entity>()){
                remap[prototype] = clone(registry, prefab.prototype, prototype);
            }

            auto translate = [&remap](entt::entity prototype){
                auto match = remap.find(prototype);
                return match != remap.end() ? match->second : entt::null;
            };

            for(auto [prototype, actual] : remap){
                if(GameObject* gameObject = registry.try_get<GameObject>(actual)){
                    gameObject->parent = translate(gameObject->parent);
                    gameObject->firstChild = translate(gameObject->firstChild);
                    gameObject->nextSibling = translate(gameObject->nextSibling);
                }
            }

            entt::entity root = translate(prefab.firstRoot);

            if(firstRoot == entt::null){
                firstRoot = root;
                return root;
            }

            for(entt::entity sibling = root; sibling != entt::null;){
                entt::entity next = registry.get<GameObject>(sibling).nextSibling;
                game_object_system::setParent(*this, sibling, parent);
                sibling = next;
            }

            return root;
        }

        entt::entity instantiate(const Prefab& prefab){
            return instantiate(prefab, entt::null);
        }

        void clear(){
            registry.clear();
            firstRoot = entt::null;
            renderList.clear();
            resources.clear();
        }
    };

    namespace ecs_manager{
        void initialize();
        void initializeComponents();
    }
}