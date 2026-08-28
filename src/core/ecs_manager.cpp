#include "ecs_manager.hpp"

namespace engine{
    namespace ecs_manager{
        void initialize(){
            initializeComponents();
        }
        
        void initializeComponents(){
            entt::meta_factory<GameObject>()
                .type("Game Object")
                .data<&GameObject::name>("Name")
                .data<&GameObject::active>("Active")
                .data<&GameObject::position>("Position")
                .data<&GameObject::rotation>("Rotation")
                .data<&GameObject::scale>("Scale");
        }
    }
}