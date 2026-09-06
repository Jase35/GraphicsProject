#include "asset_loading.hpp"

#include <filesystem>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace asset_loading{
    Prefab importRootEntity_fromGLB(
        const std::filesystem::path& path,
        Resources& resources,
        Handle<Shader> shader
    ){
        fastgltf::Parser parser;
        auto dataExpected = fastgltf::GltfDataBuffer::FromPath(path);

        if(dataExpected.error() != fastgltf::Error::None){
            std::cerr << "Failed to read file: " << path << "\n";
            return {};
        }

        auto assetExpected = parser.loadGltf(
            dataExpected.get(), path.parent_path(),
            fastgltf::Options::LoadExternalBuffers | fastgltf::Options::LoadExternalImages
        );

        if(!assetExpected){
            std::cerr << "Failed to parse glTF: " << path << "\n";
            return {};
        }

        Prefab prefab{};

        prefab.firstRoot = prefab.prototype.create();
        prefab.prototype.emplace<GameObject>(prefab.firstRoot, "Prefab");

        fastgltf::Asset& asset = assetExpected.get();

        std::vector<Image> images;
        images.reserve(asset.images.size());

        auto decodeFromMemory = [](const std::byte* bytes, size_t size) -> Image {
            int width, height, channels;
            unsigned char* data = stbi_load_from_memory(
                reinterpret_cast<const stbi_uc*>(bytes), 
                static_cast<int>(size), 
                &width, 
                &height, 
                &channels, 
                4
            );
            return data ? Image(width, height, data) : Image{};
        };

        for(const auto& image : asset.images){
            Image decoded{};

            std::visit(fastgltf::visitor{
                [](const auto&){},
                [&](const fastgltf::sources::Array& array){
                    decoded = decodeFromMemory(array.bytes.data(), array.bytes.size());
                },
                [&](const fastgltf::sources::Vector& vector){
                    decoded = decodeFromMemory(vector.bytes.data(), vector.bytes.size());
                },
                [&](const fastgltf::sources::BufferView& view){
                    const auto& bufferView = asset.bufferViews[view.bufferViewIndex];
                    const auto& buffer = asset.buffers[bufferView.bufferIndex];

                    std::visit(fastgltf::visitor{
                        [](const auto&){},
                        [&](const fastgltf::sources::Array& array){
                            decoded = decodeFromMemory(array.bytes.data() + bufferView.byteOffset, bufferView.byteLength);
                        },
                        [&](const fastgltf::sources::Vector& vector){
                            decoded = decodeFromMemory(vector.bytes.data() + bufferView.byteOffset, bufferView.byteLength);
                        },
                    }, buffer.data);
                },
                [&](const fastgltf::sources::URI& uri){
                    if(uri.uri.isLocalPath()){
                        int width, height, channels;
                        std::filesystem::path imagePath = path.parent_path() / uri.uri.fspath();
                        unsigned char* data = stbi_load(imagePath.string().c_str(), &width, &height, &channels, 4);
                        if(data){
                            decoded = Image(width, height, data);
                        }
                    }
                }
            }, image.data);

            if(!decoded.data){
                printf("Failed to load image.\n");
            }

            images.push_back(decoded);
        }

        std::vector<Handle<Material>> materials;

        for(const auto& mat : asset.materials){
            auto& color = mat.pbrData.baseColorFactor;

            Image textureImage{};
            if(mat.pbrData.baseColorTexture.has_value()){
                size_t textureIndex = mat.pbrData.baseColorTexture->textureIndex;

                if(textureIndex < asset.textures.size()){
                    const auto& texture = asset.textures[textureIndex];
                    if(texture.imageIndex.has_value() && texture.imageIndex.value() < images.size()){
                        textureImage = images[texture.imageIndex.value()];
                    }
                }
            }

            Handle<Material> material = resources.materials.add(Material(
                shader,
                vec4(color[0], color[1], color[2], color[3]),
                textureImage
            ));

            materials.push_back(material);
        }

        if(materials.empty()){
            materials.push_back(resources.materials.add(Material(
                shader,
                vec4{1.0f, 1.0f, 1.0f, 1.0f},
                Image{}
            )));
        }
        // TODO add constant for no material.

        size_t nodeCount = asset.nodes.size();
        if(nodeCount > 0){
            entt::entity boneRoot = prefab.prototype.create();
            prefab.prototype.emplace<GameObject>(boneRoot, "Bone Root");
            game_object_system::setParent(prefab.prototype, prefab.firstRoot, boneRoot, prefab.firstRoot);

            std::unordered_map<size_t, entt::entity> bones;

            for(const fastgltf::Skin& skin : asset.skins){
                size_t jointCount = skin.joints.size();
                size_t nodeCount = asset.nodes.size();

                for(size_t i = 0; i < jointCount; i++){
                    const auto& joint = skin.joints[i];
                    const auto& node = asset.nodes[joint];

                    entt::entity entity = prefab.prototype.create();
                    prefab.prototype.emplace<GameObject>(entity, node.name.c_str());
                    game_object_system::setParent(prefab.prototype, prefab.firstRoot, entity, boneRoot);

                    bones[joint] = entity;
                }

                for(size_t i = 0; i < jointCount; i++){
                    const auto& joint = skin.joints[i];
                    const auto& node = asset.nodes[joint];

                    for(size_t child : node.children){
                        auto childIt = bones.find(child);
                        if(childIt == bones.end()){
                            continue;
                        }
                        game_object_system::setParent(
                            prefab.prototype,
                            prefab.firstRoot,
                            childIt->second,
                            bones[joint]
                        );
                    }
                }

                for(size_t i = 0; i < jointCount; i++){
                    const auto& joint = skin.joints[i];
                    const auto& node = asset.nodes[joint];

                    entt::entity entity = bones[joint];
                    GameObject& gameObject = prefab.prototype.get<GameObject>(entity);

                    std::visit(fastgltf::visitor{
                        [&](const fastgltf::TRS& trs){
                            gameObject.position = vec3(
                                trs.translation[0],
                                trs.translation[1],
                                trs.translation[2]
                            );
                            gameObject.rotation = quaternionToEuler(quat(
                                trs.rotation[0],
                                trs.rotation[1],
                                trs.rotation[2],
                                trs.rotation[3]
                            ));
                            gameObject.scale = vec3(
                                trs.scale[0],
                                trs.scale[1],
                                trs.scale[2]
                            );
                        },
                        [&](const fastgltf::math::fmat4x4& mat){
                            mat4 m = make_mat4(mat.data());
                            gameObject.position = vec3(m[3]);
                            gameObject.scale = vec3(
                                length(vec3(m[0])),
                                length(vec3(m[1])),
                                length(vec3(m[2]))
                            );
                            mat3 rotMat(
                                vec3(m[0]) / gameObject.scale.x,
                                vec3(m[1]) / gameObject.scale.y,
                                vec3(m[2]) / gameObject.scale.z
                            );
                            quat q = quat_cast(rotMat);
                            gameObject.rotation = quaternionToEuler(q);
                        }
                    }, node.transform);
                }
            }

            /*for(size_t i = 0; i < nodeCount; i++){
                if(bones.find(i) != bones.end()){
                    const auto& node = asset.nodes[i];

                    for(size_t child : asset.nodes[i].children){
                        game_object_system::setParent(
                            prefab.prototype,
                            prefab.firstRoot,
                            bones[child],
                            bones[i]
                        );
                    }
                }
            }*/
        }

        size_t meshCount = asset.meshes.size();
        for(size_t mi = 0; mi < meshCount; mi++){
            const auto& mesh = asset.meshes[mi];

            entt::entity meshRoot = prefab.firstRoot;
            if(meshCount > 1){
                meshRoot = prefab.prototype.create();
                prefab.prototype.emplace<GameObject>(meshRoot, ("Mesh (" + std::to_string(mi) + ")").c_str());
                game_object_system::setParent(prefab.prototype, prefab.firstRoot, meshRoot, prefab.firstRoot);
            }

            size_t primCount = mesh.primitives.size();
            for(size_t pi = 0; pi < primCount; pi++){
                const auto& prim = mesh.primitives[pi];

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

                Handle<Mesh> meshHandle = resources.meshes.add(Mesh(vertices, indices));

                Handle<Material> material = materials[0];
                if(prim.materialIndex.has_value() && prim.materialIndex.value() < asset.materials.size()){
                    material = materials[prim.materialIndex.value()];
                }

                entt::entity entity = meshRoot;
                if(primCount > 1){
                    entity = prefab.prototype.create();
                    prefab.prototype.emplace<GameObject>(entity, ("Primitive (" + std::to_string(pi) + ")").c_str());
                    game_object_system::setParent(prefab.prototype, prefab.firstRoot, entity, meshRoot);
                }
                prefab.prototype.emplace<Renderer>(entity, meshHandle, material);
            }
        }

        return prefab;
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

    vec3 quaternionToEuler(quat quaternion){
        mat3 m = mat3_cast(quaternion);
        float sinx = clamp(-m[2][1], -1.0f, 1.0f);
        float rx = asin(sinx), ry, rz;

        if(abs(sinx) < 0.999f){
            ry = atan(m[2][0], m[2][2]);
            rz = atan(m[0][1], m[1][1]);
        }else{
            ry = atan(-m[0][2], m[0][0]);
            rz = 0.0f;
        }

        return vec3(rx, ry, rz);
    }
}