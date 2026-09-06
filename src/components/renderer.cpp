#include "renderer.hpp"

#include "core/ecs_manager.hpp"

namespace engine{

    Renderer::Renderer(Handle<Mesh> mesh, Handle<Material> material) : mesh(mesh), material(material) {}

    namespace render_system{
        namespace{
            GLuint cameraUBO = 0;
            GLuint bonesSSBO = 0;
            GLuint instancesSSBO = 0;
            size_t instanceCapacity = 0;
            std::vector<InstanceData> instancesData;

            std::vector<uint64_t> sortedKeys, sortedKeysSwap;
            std::vector<uint32_t> sortedIndices, sortedIndicesSwap;

            constexpr uint32_t PASS_COUNT = 8;

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

        void render(Scene& scene, const float& aspectRatio){
            mat4 viewMatrix = mat4(1.0f);
            mat4 projectionMatrix = mat4(1.0f);
            bool found = false;
            for(auto [entity, camera, gameObject] : scene.registry.view<Camera, GameObject>().each()){
                if(gameObject.active){ // TODO change condition.

                    viewMatrix = inverse(gameObject.globalTransform);

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
                glGenBuffers(1, &cameraUBO);
                glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
                glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(mat4), NULL, GL_DYNAMIC_DRAW);

                glGenBuffers(1, &bonesSSBO);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, bonesSSBO);
                glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_BONES * sizeof(mat4), NULL, GL_DYNAMIC_DRAW);

                glGenBuffers(1, &instancesSSBO);
            }
            const mat4 cameraMatrices[2] = { viewMatrix, projectionMatrix };

            glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(cameraMatrices), cameraMatrices);
            glBindBufferBase(GL_UNIFORM_BUFFER, 0, cameraUBO);

            std::array<mat4, MAX_BONES> boneMatrices;
            boneMatrices.fill(mat4(1.0f));

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, bonesSSBO);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, boneMatrices.size() * sizeof(mat4), boneMatrices.data());
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bonesSSBO);

            sortRenderList(scene.renderList.keys);
            const size_t renderCount = sortedKeys.size();

            if(renderCount == 0){
                return;
            }

            const std::vector<mat4>& transforms = scene.renderList.transforms;
            instancesData.resize(renderCount);
            for(size_t i = 0; i < renderCount; i++){
                const mat4& transform = transforms[sortedIndices[i]];
                InstanceData& instance = instancesData[i];
                instance.modelMatrix = transform;

                const vec3 col0(transform[0]), col1(transform[1]), col2(transform[2]);
                const vec3 cofactor = cross(col1, col2);
                const float inverseDeterminant = 1.0f / dot(col0, cofactor);
                instance.normalMatrix[0] = vec4(cofactor * inverseDeterminant, 0.0f);
                instance.normalMatrix[1] = vec4(cross(col2, col0) * inverseDeterminant, 0.0f);
                instance.normalMatrix[2] = vec4(cross(col0, col1) * inverseDeterminant, 0.0f);
            }

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, instancesSSBO);
            if(renderCount > instanceCapacity){
                instanceCapacity = renderCount;
                glBufferData(GL_SHADER_STORAGE_BUFFER, instanceCapacity * sizeof(InstanceData), NULL, GL_DYNAMIC_DRAW);
            }
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, renderCount * sizeof(InstanceData), instancesData.data());
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, instancesSSBO);
            
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
            glDeleteBuffers(1, &cameraUBO);
            glDeleteBuffers(1, &bonesSSBO);
            glDeleteBuffers(1, &instancesSSBO);
            cameraUBO = 0;
            bonesSSBO = 0;
            instancesSSBO = 0;
            instanceCapacity = 0;
            instancesData.clear();
            instancesData.shrink_to_fit();
            sortedKeys = {};
            sortedKeysSwap = {};
            sortedIndices = {};
            sortedIndicesSwap = {};
        }
    }
}