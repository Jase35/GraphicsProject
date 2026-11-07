#include "mesh.hpp"

namespace engine{
    namespace{
        struct BuildTriangle{
            uint32_t i0, i1, i2;
            vec3 centroid;
            vec3 boundMin, boundMax;
        };

        void buildNode(
            std::vector<BVHNode>& nodes,
            std::vector<BuildTriangle>& triangles,
            uint32_t nodeIndex,
            uint32_t first,
            uint32_t count,
            uint32_t depth
        ){
            vec3 boundMin(std::numeric_limits<float>::max());
            vec3 boundMax(std::numeric_limits<float>::lowest());
            vec3 centroidMin(std::numeric_limits<float>::max());
            vec3 centroidMax(std::numeric_limits<float>::lowest());

            for(uint32_t i = first; i < first + count; i++){
                const BuildTriangle& triangle = triangles[i];

                boundMin = glm::min(boundMin, triangle.boundMin);
                boundMax = glm::max(boundMax, triangle.boundMax);
                centroidMin = glm::min(centroidMin, triangle.centroid);
                centroidMax = glm::max(centroidMax, triangle.centroid);
            }

            nodes[nodeIndex].boundMin = boundMin;
            nodes[nodeIndex].boundMax = boundMax;

            const vec3 extent = centroidMax - centroidMin;
            const int axis = extent.x > extent.y
                ? (extent.x > extent.z ? 0 : 2)
                : (extent.y > extent.z ? 1 : 2);

            if(count <= BVH_LEAF_SIZE || depth >= BVH_MAX_DEPTH || extent[axis] <= 0.0f){
                nodes[nodeIndex].leftOrStart = first;
                nodes[nodeIndex].count = count;
                return;
            }

            const uint32_t mid = first + (count / 2);
            std::nth_element(
                triangles.begin() + first,
                triangles.begin() + mid,
                triangles.begin() + first + count,
                [axis](const BuildTriangle& left, const BuildTriangle& right){
                    return left.centroid[axis] < right.centroid[axis];
                }
            );

            const uint32_t left = static_cast<uint32_t>(nodes.size());
            nodes.resize(left + 2);
            nodes[nodeIndex].leftOrStart = left;
            nodes[nodeIndex].count = 0;

            buildNode(nodes, triangles, left, first, mid - first, depth + 1);
            buildNode(nodes, triangles, left + 1, mid, first + count - mid, depth + 1);
        }
    }

    Mesh::Mesh(
        const std::vector<Vertex>& vertices,
        const std::vector<uint32_t>& indices
    ) : vertices(vertices), indices(indices), indexCount(static_cast<GLsizei>(indices.size())) {
        recalculateBounds();
        buildBVH(); // Before ebo upload.

        printf(("\nsize: " + std::to_string(indices.size() / 3) + "\n").c_str());

        glGenVertexArrays(1, &vao); // Before vbo.
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
        nodes(std::move(other.nodes)),
        indexCount(other.indexCount),
        vao(other.vao),
        vbo(other.vbo),
        ebo(other.ebo),
        boundMin(other.boundMin),
        boundMax(other.boundMax)
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
            nodes = std::move(other.nodes);
            boundMin = other.boundMin;
            boundMax = other.boundMax;
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

    void Mesh::recalculateBounds(){
        if(vertices.empty()){
            boundMin = vec3(0.0f);
            boundMax = vec3(0.0f);
            return;
        }
        float px = std::numeric_limits<float>::lowest();
        float nx = std::numeric_limits<float>::max();
        float py = std::numeric_limits<float>::lowest();
        float ny = std::numeric_limits<float>::max();
        float pz = std::numeric_limits<float>::lowest();
        float nz = std::numeric_limits<float>::max();

        for(int i = 0; i < vertices.size(); i++){
            Vertex& vertex = vertices[i];

            px = glm::max(px, vertex.position.x);
            nx = glm::min(nx, vertex.position.x);
            py = glm::max(py, vertex.position.y);
            ny = glm::min(ny, vertex.position.y);
            pz = glm::max(pz, vertex.position.z);
            nz = glm::min(nz, vertex.position.z);
        }

        boundMin = vec3(nx, ny, nz);
        boundMax = vec3(px, py, pz);
    }

    void Mesh::buildBVH(){
        nodes.clear();

        const uint32_t triangleCount = static_cast<uint32_t>(indices.size() / 3);
        if(triangleCount == 0){
            return;
        }

        std::vector<BuildTriangle> triangles(triangleCount);
        for(uint32_t i = 0; i < triangleCount; i++){
            BuildTriangle& triangle = triangles[i];
            triangle.i0 = indices[i * 3];
            triangle.i1 = indices[(i * 3) + 1];
            triangle.i2 = indices[(i * 3) + 2];

            if(triangle.i0 >= vertices.size() || triangle.i1 >= vertices.size() || triangle.i2 >= vertices.size()){
                return;
            }

            const vec3 p0 = vertices[triangle.i0].position;
            const vec3 p1 = vertices[triangle.i1].position;
            const vec3 p2 = vertices[triangle.i2].position;

            triangle.boundMin = glm::min(p0, glm::min(p1, p2));
            triangle.boundMax = glm::max(p0, glm::max(p1, p2));
            triangle.centroid = (p0 + p1 + p2) / 3.0f;
        }

        nodes.reserve(2 * triangleCount);
        nodes.resize(1);
        buildNode(nodes, triangles, 0, 0, triangleCount, 0);

        for(uint32_t i = 0; i < triangleCount; i++){
            indices[i * 3] = triangles[i].i0;
            indices[(i * 3) + 1] = triangles[i].i1;
            indices[(i * 3) + 2] = triangles[i].i2;
        }
    }
}