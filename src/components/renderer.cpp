#include "renderer.hpp"

namespace engine{

    Renderer::Renderer(const Mesh& mesh) : mesh(mesh) {
        glGenVertexArrays(1, &vao); // Do before vbo.
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        // Change to GL_DYNAMIC if the vertices are modifies a lot.
        glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(uint32_t), mesh.indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, textureUV));
        glEnableVertexAttribArray(2);

        glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, boneIndices));
        glEnableVertexAttribArray(3);

        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, boneWeights));
        glEnableVertexAttribArray(4);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    Renderer::Renderer() : Renderer(Mesh::testMesh()) {}

    Renderer::~Renderer(){
        // TODO
    }

    namespace render_system{
        GLuint loadShader(
            const std::filesystem::path& vertPath,
            const std::filesystem::path& fragPath
        ){
            std::string vertexShaderSource = render_system::readFile(vertPath);
            std::string fragmentShaderSource = render_system::readFile(fragPath);

            const GLchar* vertexShaderCode = vertexShaderSource.c_str();
            const GLchar* fragmentShaderCode = fragmentShaderSource.c_str();
            
            GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertexShader, 1, &vertexShaderCode, NULL);
            glCompileShader(vertexShader);

            GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
            glCompileShader(fragmentShader);

            GLuint shaderProgram = glCreateProgram();
            glAttachShader(shaderProgram, vertexShader);
            glAttachShader(shaderProgram, fragmentShader);
            glLinkProgram(shaderProgram);

            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);

            return shaderProgram;
        }

        void render(entt::registry& registry, const float& aspectRatio){
            mat4 viewMatrix = mat4(1.0f);
            mat4 projectionMatrix = mat4(1.0f);
            bool found = false;
            for(auto [entity, camera, gameObject] : registry.view<Camera, GameObject>().each()){
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

            GLuint shaderProgram = tempShader; // TODO change.
            glUseProgram(shaderProgram);

            GLint viewLocation = glGetUniformLocation(shaderProgram, "view");
            glUniformMatrix4fv(viewLocation, 1, GL_FALSE, value_ptr(viewMatrix));

            GLint projectionLocation = glGetUniformLocation(shaderProgram, "projection");
            glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, value_ptr(projectionMatrix));

            mat4 boneMatrices[128]; // TODO

            GLuint bonesSSBO;
            glGenBuffers(1, &bonesSSBO);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, bonesSSBO);
            glBufferData(
                GL_SHADER_STORAGE_BUFFER,
                sizeof(boneMatrices),
                boneMatrices,
                GL_DYNAMIC_DRAW
            );
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bonesSSBO);

            for(auto [entity, renderer, gameObject] : registry.view<const Renderer, const GameObject>().each()){
                GLint modelMatrixLocation = glGetUniformLocation(shaderProgram, "modelMatrix");
                glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, value_ptr(gameObject.globalTransform));

                GLint normalMatrixLocation = glGetUniformLocation(shaderProgram, "normalMatrix");
                mat3 normalMatrix = mat3(transpose(inverse(gameObject.globalTransform)));
                glUniformMatrix3fv(normalMatrixLocation, 1, GL_FALSE, value_ptr(normalMatrix));

                glBindVertexArray(renderer.vao);
                glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(renderer.mesh.indices.size()), GL_UNSIGNED_INT, 0);
            }
        }

        std::string readFile(const std::filesystem::path& path){
            std::ifstream f(path.string(), std::ios::in | std::ios::binary);
            if(!f) throw std::runtime_error("Failed to open " + path.string());
            std::ostringstream ss;
            ss << f.rdbuf();
            return ss.str();
        }
    }
}