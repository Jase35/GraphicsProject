#pragma once

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <iostream>
#include <unordered_map>

#include "core/ecs_manager.hpp"
#include "components/game_object.hpp"
#include "components/renderer.hpp"
#include "composites/vertex.hpp"
#include "composites/mesh.hpp"
#include "image.hpp"

using namespace engine;

namespace asset_loading{
    Prefab importRootEntity_fromGLB(
        const std::filesystem::path& path,
        Resources& resources,
        Handle<Shader> shader
    );

    Mesh importMesh_fromFirstGLB(const std::filesystem::path& path);

    vec3 quaternionToEuler(quat quaternion);
}