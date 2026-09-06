#include "mesh.hpp"

namespace engine{
    Mesh::Mesh(
        const std::vector<Vertex>& vertices,
        const std::vector<uint32_t>& indices
    ) : vertices(vertices), indices(indices), indexCount(static_cast<GLsizei>(indices.size())) {
        glGenVertexArrays(1, &vao); // Do before vbo.
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        // Change to GL_DYNAMIC if the vertices are modifies a lot.
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

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

    Mesh::~Mesh(){
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
    }

    Mesh::Mesh(Mesh&& other) noexcept : 
        vertices(std::move(other.vertices)),
        indices(std::move(other.indices)),
        indexCount(other.indexCount),
        vao(other.vao),
        vbo(other.vbo),
        ebo(other.ebo)
    {
        other.indexCount = 0;
        other.vao = 0;
        other.vbo = 0;
        other.ebo = 0;
    }

    Mesh& Mesh::operator=(Mesh&& other) noexcept{
        if(this != &other){
            glDeleteVertexArrays(1, &vao);
            glDeleteBuffers(1, &vbo);
            glDeleteBuffers(1, &ebo);

            vertices = std::move(other.vertices);
            indices = std::move(other.indices);
            indexCount = other.indexCount;
            vao = other.vao;
            vbo = other.vbo;
            ebo = other.ebo;

            other.indexCount = 0;
            other.vao = 0;
            other.vbo = 0;
            other.ebo = 0;
        }
        return *this;
    }

    void Mesh::bind() const{
        glBindVertexArray(vao);
    }
}