#include "renderer.hpp"

#include "core/ecs_manager.hpp"

namespace engine{

    Renderer::Renderer(Handle<Mesh> mesh, Handle<Material> material) : mesh(mesh), material(material) {}

    SphereRenderer::SphereRenderer(Handle<Material> material) : material(material) {}

    namespace render_system{
        namespace{
            GLuint resolutionUBO = 0;
            GLuint cameraUBO = 0;
            GLuint bonesSSBO = 0;
            GLuint instancesSSBO = 0;
            GLuint verticesSSBO = 0;
            GLuint indicesSSBO = 0;
            GLuint BVHNodesSSBO = 0;
            GLuint spheresSSBO = 0;
            GLuint spheresVAO = 0;
            size_t instanceCapacity = 0;
            size_t sphereCapacity = 0;
            std::vector<InstanceData> instancesData;
            std::vector<GPUSphere> spheresData;

            struct SphereBatch{
                uint16_t materialId = 0;
                uint32_t first = 0;
                uint32_t count = 0;
            };

            std::vector<std::pair<uint16_t, GPUSphere>> sphereEntries;
            std::vector<SphereBatch> sphereBatches;

            struct GeometryRange{
                uint32_t vertexOffset = 0;
                uint32_t indexOffset = 0;
                uint32_t triangleCount = 0;
                uint32_t nodeOffset = 0;
                vec3 boundMin{0.0f};
                vec3 boundMax{0.0f};
            };

            std::vector<GeometryRange> meshRanges;
            uint32_t uploadedGeometryRevision = 0xFFFFFFFF;

            std::vector<uint64_t> sortedKeys, sortedKeysSwap;
            std::vector<uint32_t> sortedIndices, sortedIndicesSwap;

            constexpr uint32_t PASS_COUNT = 8;
            
            void uploadGeometry(Resources& resources){
                const size_t meshCount = resources.meshes.size();
                meshRanges.assign(meshCount, {});

                std::vector<GPUVertex> vertices;
                std::vector<uint32_t> indices;
                std::vector<BVHNode> BVHNodes;

                for(size_t i = 0; i < meshCount; i++){
                    Mesh* mesh = resources.meshes.get(static_cast<uint16_t>(i));
                    if(mesh == nullptr){
                        continue;
                    }

                    GeometryRange& range = meshRanges[i];
                    range.vertexOffset = static_cast<uint32_t>(vertices.size());
                    range.indexOffset = static_cast<uint32_t>(indices.size());
                    range.triangleCount = static_cast<uint32_t>(mesh->indices.size() / 3);
                    range.nodeOffset = static_cast<uint32_t>(BVHNodes.size());
                    range.boundMin = mesh->boundMin;
                    range.boundMax = mesh->boundMax;

                    vertices.reserve(vertices.size() + mesh->vertices.size());
                    for(const Vertex& vertex : mesh->vertices){
                        vertices.push_back({
                            vec4(vertex.position, vertex.textureUV.x),
                            vec4(vertex.normal, vertex.textureUV.y)
                        });
                    }

                    indices.insert(indices.end(), mesh->indices.begin(), mesh->indices.end());
                    BVHNodes.insert(BVHNodes.end(), mesh->nodes.begin(), mesh->nodes.end());
                }

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, verticesSSBO);
                glBufferData(
                    GL_SHADER_STORAGE_BUFFER,
                    vertices.size() * sizeof(GPUVertex),
                    vertices.data(),
                    GL_STATIC_DRAW
                );

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, indicesSSBO);
                glBufferData(
                    GL_SHADER_STORAGE_BUFFER,
                    indices.size() * sizeof(uint32_t),
                    indices.data(),
                    GL_STATIC_DRAW
                );

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, BVHNodesSSBO);
                glBufferData(
                    GL_SHADER_STORAGE_BUFFER,
                    BVHNodes.size() * sizeof(BVHNode),
                    BVHNodes.data(),
                    GL_STATIC_DRAW
                );
            }

            void uploadSpheres(Scene& scene){
                spheresData.clear();
                sphereEntries.clear();
                sphereBatches.clear();

                for(auto [entity, sphereRenderer, gameObject] : scene.registry.view<SphereRenderer, GameObject>().each()){
                    if(!gameObject.active){
                        continue;
                    }

                    const Material* material = scene.resources.materials.get(sphereRenderer.material);
                    if(!material){
                        continue;
                    }

                    const mat4& transform = gameObject.globalTransform;
                    const float radius = std::max({
                        length(vec3(transform[0])),
                        length(vec3(transform[1])),
                        length(vec3(transform[2]))
                    });

                    sphereEntries.push_back({
                        sphereRenderer.material.index,
                        {material->baseColor, vec3(transform[3]), 2.0f * radius}
                    });
                }

                std::sort(
                    sphereEntries.begin(),
                    sphereEntries.end(),
                    [](const auto& left, const auto& right){ return left.first < right.first; }
                );

                spheresData.reserve(sphereEntries.size());
                for(const auto& [materialId, sphere] : sphereEntries){
                    if(sphereBatches.empty() || sphereBatches.back().materialId != materialId){
                        sphereBatches.push_back({materialId, static_cast<uint32_t>(spheresData.size()), 0});
                    }
                    sphereBatches.back().count++;
                    spheresData.push_back(sphere);
                }

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, spheresSSBO);
                if(spheresData.size() > sphereCapacity){
                    sphereCapacity = spheresData.size();
                    glBufferData(
                        GL_SHADER_STORAGE_BUFFER,
                        sphereCapacity * sizeof(GPUSphere),
                        NULL,
                        GL_DYNAMIC_DRAW
                    );
                }
                if(!spheresData.empty()){
                    glBufferSubData(
                        GL_SHADER_STORAGE_BUFFER,
                        0,
                        spheresData.size() * sizeof(GPUSphere),
                        spheresData.data()
                    );
                }
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, spheresSSBO);
            }

            void renderSpheres(Scene& scene){
                if(sphereBatches.empty()){
                    return;
                }

                glBindVertexArray(spheresVAO);

                uint32_t lastShaderId = 0x10000;
                for(const SphereBatch& batch : sphereBatches){
                    Material* material = scene.resources.materials.get(batch.materialId);
                    if(material == nullptr){
                        continue;
                    }

                    Shader* shader = scene.resources.shaders.get(material->shader);
                    if(shader == nullptr){
                        continue;
                    }

                    if(material->shader.index != lastShaderId){
                        shader->bind();
                        lastShaderId = material->shader.index;
                    }
                    material->bind();

                    glUniform1ui(INSTANCE_OFFSET_LOCATION, batch.first);
                    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, static_cast<GLsizei>(batch.count));
                }

                glBindVertexArray(0);
            }

            void sortRenderList(const std::vector<uint64_t>& keys){
                const uint32_t count = static_cast<uint32_t>(keys.size());

                sortedKeys.resize(count);
                sortedIndices.resize(count);
                sortedKeysSwap.resize(count);
                sortedIndicesSwap.resize(count);

                uint32_t histograms[PASS_COUNT][256] = {};
                for(uint32_t i = 0; i < count; i++){
                    const uint64_t key = keys[i];
                    sortedKeys[i] = key;
                    sortedIndices[i] = i;
                    for(uint32_t pass = 0; pass < PASS_COUNT; pass++){
                        histograms[pass][(key >> (pass * 8)) & 0xFF]++;
                    }
                }

                if(count <= 1){
                    return;
                }

                for(uint32_t pass = 0; pass < PASS_COUNT; pass++){
                    const uint32_t shift = pass * 8;
                    const uint32_t* histogram = histograms[pass];

                    if(histogram[(sortedKeys[0] >> shift) & 0xFF] == count){
                        continue;
                    }

                    uint32_t offsets[256];
                    uint32_t sum = 0;

                    for(uint32_t digit = 0; digit < 256; digit++){
                        offsets[digit] = sum;
                        sum += histogram[digit];
                    }

                    for(uint32_t i = 0; i < count; i++){
                        const uint32_t destination = offsets[(sortedKeys[i] >> shift) & 0xFF]++;
                        sortedKeysSwap[destination] = sortedKeys[i];
                        sortedIndicesSwap[destination] = sortedIndices[i];
                    }

                    sortedKeys.swap(sortedKeysSwap);
                    sortedIndices.swap(sortedIndicesSwap);
                }
            }
        }

        void render(Scene& scene, const ivec2& resolution, const float& aspectRatio){
            mat4 viewMatrix = mat4(1.0f);
            mat4 projectionMatrix = mat4(1.0f);
            vec4 cameraPosition = vec4(0.0f, 0.0f, 0.0f, 1.0f);
            bool found = false;
            for(auto [entity, camera, gameObject] : scene.registry.view<Camera, GameObject>().each()){
                if(gameObject.active){ // TODO change condition.

                    viewMatrix = inverse(gameObject.globalTransform);
                    cameraPosition = vec4(vec3(gameObject.globalTransform[3]), 1.0f);

                    float fovTan = std::tan(camera.fov / 2.0f);
                    projectionMatrix = mat4(
                        vec4(
                            1.0f / (aspectRatio * fovTan),
                            0.0f, 0.0f, 0.0f
                        ),
                        vec4(
                            0.0f,
                            1.0f / fovTan,
                            0.0f, 0.0f
                        ),
                        vec4(
                            0.0f, 0.0f,
                            -((camera.far + camera.near) / (camera.far - camera.near)),
                            -1.0f
                        ),
                        vec4(
                            0.0f, 0.0f,
                            -((2.0f * camera.far * camera.near) / (camera.far - camera.near)),
                            0.0f
                        )
                    );

                    found = true;
                    break;
                }
            }

            if(!found){
                // TODO print a warning or something.
                return;
            }
            
            if(cameraUBO == 0){
                glGenBuffers(1, &resolutionUBO);
                glBindBuffer(GL_UNIFORM_BUFFER, resolutionUBO);
                glBufferData(GL_UNIFORM_BUFFER, sizeof(ivec4), NULL, GL_DYNAMIC_DRAW);

                glGenBuffers(1, &cameraUBO);
                glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
                glBufferData(GL_UNIFORM_BUFFER, (2 * sizeof(mat4)) + sizeof(vec4), NULL, GL_DYNAMIC_DRAW);

                glGenBuffers(1, &bonesSSBO);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, bonesSSBO);
                glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_BONES * sizeof(mat4), NULL, GL_DYNAMIC_DRAW);

                glGenBuffers(1, &instancesSSBO);
                glGenBuffers(1, &verticesSSBO);
                glGenBuffers(1, &indicesSSBO);
                glGenBuffers(1, &spheresSSBO);
                glGenBuffers(1, &BVHNodesSSBO);
                glGenVertexArrays(1, &spheresVAO);
            }
            const mat4 cameraMatrices[2] = { viewMatrix, projectionMatrix };

            glBindBuffer(GL_UNIFORM_BUFFER, resolutionUBO);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ivec2), value_ptr(resolution));
            glBindBufferBase(GL_UNIFORM_BUFFER, 0, resolutionUBO);

            glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(cameraMatrices), cameraMatrices);
            glBufferSubData(GL_UNIFORM_BUFFER, sizeof(cameraMatrices), sizeof(vec4), value_ptr(cameraPosition));
            glBindBufferBase(GL_UNIFORM_BUFFER, 1, cameraUBO);

            std::array<mat4, MAX_BONES> boneMatrices;
            boneMatrices.fill(mat4(1.0f));

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, bonesSSBO);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, boneMatrices.size() * sizeof(mat4), boneMatrices.data());
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, bonesSSBO);

            if(uploadedGeometryRevision != scene.resources.meshes.revision()){
                uploadedGeometryRevision = scene.resources.meshes.revision();
                uploadGeometry(scene.resources);
            }

            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, verticesSSBO);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, indicesSSBO);

            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, BVHNodesSSBO);

            uploadSpheres(scene);

            const uint32_t sphereCount = static_cast<uint32_t>(spheresData.size());
            glBindBuffer(GL_UNIFORM_BUFFER, resolutionUBO);
            glBufferSubData(GL_UNIFORM_BUFFER, sizeof(ivec2) + sizeof(uint32_t), sizeof(uint32_t), &sphereCount);

            sortRenderList(scene.renderList.keys);
            const size_t renderCount = sortedKeys.size();

            const std::vector<mat4>& transforms = scene.renderList.transforms;
            instancesData.resize(renderCount);
            for(size_t i = 0; i < renderCount; i++){
                const mat4& transform = transforms[sortedIndices[i]];
                InstanceData& instance = instancesData[i];
                instance.modelMatrix = transform;
                instance.inverseModelMatrix = inverse(transform);

                const vec3 col0(transform[0]), col1(transform[1]), col2(transform[2]);
                const vec3 cofactor = cross(col1, col2);
                const float inverseDeterminant = 1.0f / dot(col0, cofactor);
                instance.normalMatrix[0] = vec4(cofactor * inverseDeterminant, 0.0f);
                instance.normalMatrix[1] = vec4(cross(col2, col0) * inverseDeterminant, 0.0f);
                instance.normalMatrix[2] = vec4(cross(col0, col1) * inverseDeterminant, 0.0f);

                const uint16_t meshId = static_cast<uint16_t>(sortedKeys[i] >> 8);
                if(meshId < meshRanges.size()){
                    const GeometryRange& range = meshRanges[meshId];
                    instance.geometry = uvec4(range.vertexOffset, range.indexOffset, range.triangleCount, range.nodeOffset);

                    const vec3 localCenter = 0.5f * (range.boundMin + range.boundMax);
                    const vec3 localExtent = 0.5f * (range.boundMax - range.boundMin);
                    const vec3 worldCenter = vec3(transform * vec4(localCenter, 1.0f));
                    const vec3 worldExtent =
                        (abs(col0) * localExtent.x) +
                        (abs(col1) * localExtent.y) +
                        (abs(col2) * localExtent.z);

                    instance.boundMin = vec4(worldCenter - worldExtent, 0.0f);
                    instance.boundMax = vec4(worldCenter + worldExtent, 0.0f);
                }else{
                    instance.geometry = uvec4(0);
                    instance.boundMin = vec4(0.0f);
                    instance.boundMax = vec4(0.0f);
                }
            }

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, instancesSSBO);
            if(renderCount > instanceCapacity){
                instanceCapacity = renderCount;
                glBufferData(GL_SHADER_STORAGE_BUFFER, instanceCapacity * sizeof(InstanceData), NULL, GL_DYNAMIC_DRAW);
            }
            if(renderCount > 0){
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, renderCount * sizeof(InstanceData), instancesData.data());
            }
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, instancesSSBO);

            const uint32_t instanceCount = static_cast<uint32_t>(renderCount);
            glBindBuffer(GL_UNIFORM_BUFFER, resolutionUBO);
            glBufferSubData(GL_UNIFORM_BUFFER, sizeof(ivec2), sizeof(uint32_t), &instanceCount);
            
            renderSpheres(scene);

            if(renderCount == 0){
                return;
            }
            
            uint32_t lastShaderId = 0x10000, lastMaterialId = 0x10000, lastMeshId = 0x10000;
            GLsizei indicesSize = 0;
            for(size_t start = 0; start < renderCount;){
                const uint64_t batchKey = sortedKeys[start] >> 8;
                size_t end = start + 1;
                while(end < renderCount && (sortedKeys[end] >> 8) == batchKey){
                    end++;
                }

                //uint16_t layerId = static_cast<uint16_t>(batchKey >> 56);
                uint16_t shaderId = static_cast<uint16_t>(batchKey >> 32);
                uint16_t materialId = static_cast<uint16_t>(batchKey >> 16);
                uint16_t meshId = static_cast<uint16_t>(batchKey);

                if(shaderId != lastShaderId){
                    Shader* shader = scene.resources.shaders.get(shaderId);
                    if(shader == nullptr){
                        start = end;
                        continue;
                    }
                    shader->bind();

                    lastShaderId = shaderId;
                }

                if(materialId != lastMaterialId){
                    Material* material = scene.resources.materials.get(materialId);
                    if(material == nullptr){
                        start = end;
                        continue;
                    }
                    material->bind();
                    
                    lastMaterialId = materialId;
                }

                if(meshId != lastMeshId){
                    Mesh* mesh = scene.resources.meshes.get(meshId);
                    if(mesh == nullptr){
                        start = end;
                        continue;
                    }
                    mesh->bind();

                    indicesSize = mesh->indexCount;

                    lastMeshId = meshId;
                }

                glUniform1ui(
                    INSTANCE_OFFSET_LOCATION, 
                    static_cast<GLuint>(start)
                );

                glDrawElementsInstanced(
                    GL_TRIANGLES,
                    indicesSize,
                    GL_UNSIGNED_INT,
                    nullptr,
                    static_cast<GLsizei>(end - start)
                );

                start = end;
            }
        }

        void shutdown(){
            glDeleteBuffers(1, &resolutionUBO);
            glDeleteBuffers(1, &cameraUBO);
            glDeleteBuffers(1, &bonesSSBO);
            glDeleteBuffers(1, &instancesSSBO);
            glDeleteBuffers(1, &verticesSSBO);
            glDeleteBuffers(1, &indicesSSBO);
            glDeleteBuffers(1, &spheresSSBO);
            glDeleteBuffers(1, &BVHNodesSSBO);
            glDeleteVertexArrays(1, &spheresVAO);
            resolutionUBO = 0;
            cameraUBO = 0;
            bonesSSBO = 0;
            instancesSSBO = 0;
            verticesSSBO = 0;
            indicesSSBO = 0;
            spheresSSBO = 0;
            BVHNodesSSBO = 0;
            spheresVAO = 0;
            instanceCapacity = 0;
            sphereCapacity = 0;
            instancesData.clear();
            instancesData.shrink_to_fit();
            spheresData.clear();
            spheresData.shrink_to_fit();
            sphereEntries.clear();
            sphereEntries.shrink_to_fit();
            sphereBatches.clear();
            sphereBatches.shrink_to_fit();
            meshRanges.clear();
            meshRanges.shrink_to_fit();
            uploadedGeometryRevision = 0xFFFFFFFF;
            sortedKeys = {};
            sortedKeysSwap = {};
            sortedIndices = {};
            sortedIndicesSwap = {};
        }
    }
}