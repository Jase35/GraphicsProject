#include "asset_loading.hpp"

namespace asset_loading{
    entt::entity importRootEntity_fromWholeGLB(Scene& scene, const std::filesystem::path& path){
        fastgltf::Parser parser;
        auto dataExpected = fastgltf::GltfDataBuffer::FromPath(path);

        if(dataExpected.error() != fastgltf::Error::None){
            std::cerr << "Failed to read file: " << path << "\n";
            return entt::null;
        }

        auto assetExpected = parser.loadGltf(
            dataExpected.get(), path.parent_path(),
            fastgltf::Options::LoadExternalBuffers | fastgltf::Options::LoadExternalImages
        );

        if(!assetExpected){
            std::cerr << "Failed to parse glTF: " << path << "\n";
            return entt::null;
        }

        entt::entity result = scene.registry.create();
        scene.registry.emplace<GameObject>(result, "Prefab");

        fastgltf::Asset& asset = assetExpected.get();

        for(size_t mi = 0; mi < asset.meshes.size(); mi++){
            const auto& mesh = asset.meshes[mi];
            size_t primIndex = 0;
            for(const auto& prim : mesh.primitives){

                const auto& posAccessor = asset.accessors[prim.findAttribute("POSITION")->accessorIndex];
                std::vector<vec3> positions;
                positions.reserve(posAccessor.count);
                fastgltf::iterateAccessor<vec3>(
                    asset, posAccessor,
                    [&](vec3 pos){ positions.push_back(pos); }
                );

                std::vector<vec3> normals;
                auto normIt = prim.findAttribute("NORMAL");
                if(normIt != prim.attributes.end()){
                    const auto& normAccessor = asset.accessors[normIt->accessorIndex];
                    normals.reserve(normAccessor.count);
                    fastgltf::iterateAccessor<vec3>(
                        asset, normAccessor,
                        [&](vec3 n){ normals.push_back(n); }
                    );
                }else{
                    normals.resize(positions.size(), vec3(0.0f, 0.0f, 1.0f));
                }

                std::vector<vec2> textureUVs;
                auto uvIt = prim.findAttribute("TEXCOORD_0");
                if(uvIt != prim.attributes.end()){
                    const auto& uvAccessor = asset.accessors[uvIt->accessorIndex];
                    textureUVs.reserve(uvAccessor.count);
                    fastgltf::iterateAccessor<vec2>(
                        asset, uvAccessor,
                        [&](vec2 uv){ textureUVs.push_back(uv); }
                    );
                }else{
                    textureUVs.resize(positions.size(), vec2(0.0f, 0.0f));
                }

                size_t vertexCount = positions.size();
                std::vector<Vertex> vertices(vertexCount);

                for(size_t i = 0; i < vertexCount; i++){
                    vertices[i].position = positions[i];
                    vertices[i].normal = normals[i];
                    vertices[i].textureUV = textureUVs[i];
                }

                auto jointsIt = prim.findAttribute("JOINTS_0");
                if(jointsIt != prim.attributes.end()){
                    const auto& jointsAccessor = asset.accessors[jointsIt->accessorIndex];
                    fastgltf::iterateAccessor<glm::u32vec4>(
                        asset, jointsAccessor,
                        [&](glm::u32vec4 j){
                            // TODO
                        }
                    );
                }
                
                auto weightsIt = prim.findAttribute("WEIGHTS_0");
                if(weightsIt != prim.attributes.end()){
                    const auto& weightsAccessor = asset.accessors[weightsIt->accessorIndex];
                    size_t wi = 0;
                    fastgltf::iterateAccessor<glm::vec4>(
                        asset, weightsAccessor,
                        [&](glm::vec4 w){
                            if(wi < vertices.size()){
                                vertices[wi].boneWeights = w;
                            }
                            wi++;
                        }
                    );
                }

                std::vector<uint32_t> indices;
                if(prim.indicesAccessor.has_value()){
                    const auto& indexAccessor = asset.accessors[prim.indicesAccessor.value()];
                    indices.reserve(indexAccessor.count);
                    fastgltf::iterateAccessor<uint32_t>(
                        asset, indexAccessor,
                        [&](uint32_t index){ indices.push_back(index); }
                    );
                }

                // TODO
                //engine::Mesh mesh = Mesh(vertices, indices);
                entt::entity entity = scene.registry.create();
                scene.registry.emplace<GameObject>(entity, ("Mesh (" + std::to_string(primIndex) + ")").c_str());
                //scene.registry.emplace<Renderer>(entity, mesh);
                game_object_system::setParent(scene, entity, result);
                //return entity;

                primIndex++;
            }
        }

        return result;
    }

    Mesh importMesh_fromFirstGLB(const std::filesystem::path& path){
        fastgltf::Parser parser;
        auto dataExpected = fastgltf::GltfDataBuffer::FromPath(path);

        std::vector<Vertex> emptyVertices;
        std::vector<uint32_t> emptyIndices;

        if(dataExpected.error() != fastgltf::Error::None){
            std::cerr << "Failed to read file: " << path << "\n";
            return Mesh(emptyVertices, emptyIndices);
        }

        auto assetExpected = parser.loadGltf(
            dataExpected.get(), path.parent_path(),
            fastgltf::Options::LoadExternalBuffers | fastgltf::Options::LoadExternalImages
        );

        if(!assetExpected){
            std::cerr << "Failed to parse glTF: " << path << "\n";
            return Mesh(emptyVertices, emptyIndices);
        }

        fastgltf::Asset& asset = assetExpected.get();

        for(size_t mi = 0; mi < asset.meshes.size(); mi++){
            const auto& mesh = asset.meshes[mi];
            for(const auto& prim : mesh.primitives){

                const auto& posAccessor = asset.accessors[prim.findAttribute("POSITION")->accessorIndex];
                std::vector<vec3> positions;
                positions.reserve(posAccessor.count);
                fastgltf::iterateAccessor<vec3>(
                    asset, posAccessor,
                    [&](vec3 pos){ positions.push_back(pos); }
                );

                std::vector<vec3> normals;
                auto normIt = prim.findAttribute("NORMAL");
                if(normIt != prim.attributes.end()){
                    const auto& normAccessor = asset.accessors[normIt->accessorIndex];
                    normals.reserve(normAccessor.count);
                    fastgltf::iterateAccessor<vec3>(
                        asset, normAccessor,
                        [&](vec3 n){ normals.push_back(n); }
                    );
                }else{
                    normals.resize(positions.size(), vec3(0.0f, 0.0f, 1.0f));
                }

                std::vector<vec2> textureUVs;
                auto uvIt = prim.findAttribute("TEXCOORD_0");
                if(uvIt != prim.attributes.end()){
                    const auto& uvAccessor = asset.accessors[uvIt->accessorIndex];
                    textureUVs.reserve(uvAccessor.count);
                    fastgltf::iterateAccessor<vec2>(
                        asset, uvAccessor,
                        [&](vec2 uv){ textureUVs.push_back(uv); }
                    );
                }else{
                    textureUVs.resize(positions.size(), vec2(0.0f, 0.0f));
                }

                size_t vertexCount = positions.size();
                std::vector<Vertex> vertices(vertexCount);

                for(size_t i = 0; i < vertexCount; i++){
                    vertices[i].position = positions[i];
                    vertices[i].normal = normals[i];
                    vertices[i].textureUV = textureUVs[i];
                }

                auto jointsIt = prim.findAttribute("JOINTS_0");
                if(jointsIt != prim.attributes.end()){
                    const auto& jointsAccessor = asset.accessors[jointsIt->accessorIndex];
                    fastgltf::iterateAccessor<glm::u32vec4>(
                        asset, jointsAccessor,
                        [&](glm::u32vec4 j){
                            // TODO
                        }
                    );
                }
                
                auto weightsIt = prim.findAttribute("WEIGHTS_0");
                if(weightsIt != prim.attributes.end()){
                    const auto& weightsAccessor = asset.accessors[weightsIt->accessorIndex];
                    size_t wi = 0;
                    fastgltf::iterateAccessor<glm::vec4>(
                        asset, weightsAccessor,
                        [&](glm::vec4 w){
                            if(wi < vertices.size()){
                                vertices[wi].boneWeights = w;
                            }
                            wi++;
                        }
                    );
                }

                std::vector<uint32_t> indices;
                if(prim.indicesAccessor.has_value()){
                    const auto& indexAccessor = asset.accessors[prim.indicesAccessor.value()];
                    indices.reserve(indexAccessor.count);
                    fastgltf::iterateAccessor<uint32_t>(
                        asset, indexAccessor,
                        [&](uint32_t index){ indices.push_back(index); }
                    );
                }

                return Mesh(vertices, indices);
            }
        }

        return Mesh(emptyVertices, emptyIndices);
    }
}