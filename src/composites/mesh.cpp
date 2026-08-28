#include "mesh.hpp"

namespace engine{
    Mesh::Mesh(){
        // TODO
    }

    Mesh::Mesh(
        const std::vector<Vertex>& vertices,
        const std::vector<uint32_t>& indices
    ) : vertices(vertices), indices(indices) {}

    Mesh::~Mesh(){
        // TODO
    }

    Mesh Mesh::testMesh(){
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
    }
}