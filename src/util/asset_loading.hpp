#pragma once

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <iostream>

#include "core/ecs_manager.hpp"
#include "components/game_object.hpp"
#include "components/renderer.hpp"
#include "composites/vertex.hpp"
#include "composites/mesh.hpp"

using namespace engine;

namespace asset_loading{
    entt::entity importRootEntity_fromWholeGLB(Scene& scene, const std::filesystem::path& path);
    engine::Mesh importMesh_fromFirstGLB(entt::registry& registry, const std::filesystem::path& path);

    engine::Mesh toMesh(const fastgltf::Asset& asset, const fastgltf::Primitive& prim);
}