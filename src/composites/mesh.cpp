#include "mesh.hpp"

namespace engine{
    Mesh::Mesh(
        const std::vector<Vertex>& vertices,
        const std::vector<uint32_t>& indices
    ) : vertices(vertices), indices(indices) {
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
        // TODO
    }

    void Mesh::bind() const{
        glBindVertexArray(vao);
    }

    /*Mesh Mesh::testMesh(){
        Mesh mesh;

        mesh.vertices.push_back(Vertex(vec3(0.25f, 0.25f, 0.25f)));    // A
        mesh.vertices.push_back(Vertex(vec3(-0.25f, 0.25f, 0.25f)));   // B
        mesh.vertices.push_back(Vertex(vec3(0.25f, -0.25f, 0.25f)));   // C
        mesh.vertices.push_back(Vertex(vec3(-0.25f, -0.25f, 0.25f)));  // D
        mesh.vertices.push_back(Vertex(vec3(0.25f, 0.25f, -0.25f)));   // E
        mesh.vertices.push_back(Vertex(vec3(-0.25f, 0.25f, -0.25f)));  // F
        mesh.vertices.push_back(Vertex(vec3(0.25f, -0.25f, -0.25f)));  // G
        mesh.vertices.push_back(Vertex(vec3(-0.25f, -0.25f, -0.25f))); // H

        mesh.indices.push_back(4);
        mesh.indices.push_back(5);
        mesh.indices.push_back(1);

        mesh.indices.push_back(0);
        mesh.indices.push_back(4);
        mesh.indices.push_back(1);

        mesh.indices.push_back(2);
        mesh.indices.push_back(0);
        mesh.indices.push_back(1);

        mesh.indices.push_back(3);
        mesh.indices.push_back(2);
        mesh.indices.push_back(1);

        mesh.indices.push_back(4);
        mesh.indices.push_back(0);
        mesh.indices.push_back(2);

        mesh.indices.push_back(2);
        mesh.indices.push_back(6);
        mesh.indices.push_back(4);

        mesh.indices.push_back(3);
        mesh.indices.push_back(1);
        mesh.indices.push_back(5);

        mesh.indices.push_back(7);
        mesh.indices.push_back(3);
        mesh.indices.push_back(5);

        mesh.indices.push_back(6);
        mesh.indices.push_back(2);
        mesh.indices.push_back(3);

        mesh.indices.push_back(6);
        mesh.indices.push_back(3);
        mesh.indices.push_back(7);

        mesh.indices.push_back(6);
        mesh.indices.push_back(7);
        mesh.indices.push_back(5);

        mesh.indices.push_back(6);
        mesh.indices.push_back(5);
        mesh.indices.push_back(4);

        return mesh;
    }*/
}