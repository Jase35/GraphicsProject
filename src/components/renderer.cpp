#include "renderer.hpp"

#include "core/ecs_manager.hpp"

namespace engine{

    Renderer::Renderer(Handle<Mesh> mesh, Handle<Shader> shader) : mesh(mesh), shader(shader) {
        // TODO rebuild render batching.
    }

    Renderer::~Renderer(){
        // TODO
    }

    namespace render_system{
        void render(Scene& scene, ResourcePool<Mesh>& meshes, ResourcePool<Shader>& shaders, const float& aspectRatio){
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

            static GLuint cameraUBO = 0;
            static GLuint bonesSSBO = 0;
            static GLuint instancesSSBO = 0;
            static size_t instanceCapacity = 0;
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

            /*for(auto [entity, renderer, gameObject] : registry.view<const Renderer, const GameObject>().each()){
                Mesh* mesh = meshes.get(renderer.mesh);
                Shader* shader = shaders.get(renderer.shader);
                if(mesh == nullptr || shader == nullptr){
                    continue;
                }
                
                shader->bind();

                GLint modelMatrixLocation = glGetUniformLocation(shader->shaderId, "modelMatrix");
                glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, value_ptr(gameObject.globalTransform));

                GLint normalMatrixLocation = glGetUniformLocation(shader->shaderId, "normalMatrix");
                mat3 normalMatrix = mat3(transpose(inverse(gameObject.globalTransform)));
                glUniformMatrix3fv(normalMatrixLocation, 1, GL_FALSE, value_ptr(normalMatrix));

                mesh->bind();
                glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
            }*/

            // Around 180 TPS with 1,000 Mikus prior to batching.

            std::sort(
                scene.renderList.begin(),
                scene.renderList.end(),
                [](auto& a, auto& b) { return a.key < b.key; }
            );

            static std::vector<InstanceData> instancesData;

            uint16_t lastShaderId = 0xFFFF, lastMaterialId = 0xFFFF, lastMeshId = 0xFFFF;
            GLsizei indicesSize = 0;
            for(size_t begin = 0; begin < scene.renderList.size();){
                const uint64_t batchKey = scene.renderList[begin].key >> 8;
                size_t end = begin;
                while(end < scene.renderList.size() && (scene.renderList[end].key >> 8) == batchKey){
                    end++;
                }

                //uint16_t layerId = static_cast<uint16_t>(scene.renderList[begin].key >> 56);
                uint16_t shaderId = static_cast<uint16_t>(scene.renderList[begin].key >> 40);
                uint16_t materialId = static_cast<uint16_t>(scene.renderList[begin].key >> 24);
                uint16_t meshId = static_cast<uint16_t>(scene.renderList[begin].key >> 8);

                if(shaderId != lastShaderId){
                    Shader* shader = shaders.get(shaderId);
                    if(shader == nullptr){
                        begin = end;
                        continue;
                    }
                    shader->bind();

                    lastShaderId = shaderId;
                }

                if(materialId != lastMaterialId){
                    // TODO
                    lastMaterialId = materialId;
                }

                if(meshId != lastMeshId){
                    Mesh* mesh = meshes.get(meshId);
                    if(mesh == nullptr){
                        begin = end;
                        continue;
                    }
                    mesh->bind();

                    indicesSize = static_cast<GLsizei>(mesh->indices.size());

                    lastMeshId = meshId;
                }

                instancesData.clear();
                instancesData.reserve(end - begin);
                for(size_t i = begin; i < end; i++){
                    const mat4& transform = scene.renderList[i].transform;
                    instancesData.push_back({ transform, transpose(inverse(transform)) });
                }

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, instancesSSBO);
                if(instancesData.size() > instanceCapacity){
                    instanceCapacity = instancesData.size();
                    glBufferData(GL_SHADER_STORAGE_BUFFER, instanceCapacity * sizeof(InstanceData), NULL, GL_DYNAMIC_DRAW);
                }
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, instancesData.size() * sizeof(InstanceData), instancesData.data());
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, instancesSSBO);

                glDrawElementsInstanced(
                    GL_TRIANGLES,
                    indicesSize,
                    GL_UNSIGNED_INT,
                    nullptr,
                    static_cast<GLsizei>(instancesData.size())
                );

                begin = end;
            }
        }
    }
}