#version 430 core

const int MAX_REFLECTIONS = 50;
const int MAX_TRAVERSAL_STACK = 32;
const float FLOAT_MAX = 3.402823466e+38;

layout (std140, binding = 0) uniform ResolutionBuffer{
    ivec2 windowResolution;
    uint instanceCount;
    uint sphereCount;
};

layout (std140, binding = 1) uniform CameraBuffer{
    mat4 view;
    mat4 projection;
    vec4 position;
}camera;

struct GPUVertex{
    vec4 position;
    vec4 normal;
    //vec2 uv;
    //uint materialIndex;
};

struct GPUSphere{
    vec4 baseColor; // TODO remove.
    vec3 position;
    float diameter;
    //uint materialIndex;
};

struct GPUMaterial{
    vec4 baseColor;
    sampler2D colorTexture;
};

struct InstanceData{
    mat4 modelMatrix;
    mat4 inverseModelMatrix;
    vec4 normalMatrix[3];
    uvec4 geometry;
    vec4 boundMin;
    vec4 boundMax;
};

struct BVHNode{
    vec3 boundMin;
    uint leftOrStart;
    vec3 boundMax;
    uint count; // 0 means inner node.
};

layout (std430, binding = 0) readonly buffer InstanceBuffer{
    InstanceData instances[];
};

layout (std430, binding = 3) readonly buffer VertexBuffer{
    GPUVertex vertices[];
};

layout (std430, binding = 4) readonly buffer IndexBuffer{
    uint indices[];
};

layout (std430, binding = 5) readonly buffer SphereBuffer{
    GPUSphere spheres[];
};

layout (std430, binding = 6) readonly buffer BVHNodeBuffer{
    BVHNode bvhNodes[];
};

/*layout (std430, binding = 6) readonly buffer InstanceBuffer{
    GPUInstace instances[];
};*/

/*layout (std430, binding = 6) readonly buffer MaterialBuffer{
    GPUMaterial materials[];
};*/

struct Ray{
    vec3 origin;
    vec3 direction;
    vec3 invDirection;
};

Ray createRay(vec3 origin, vec3 direction){
    Ray ray;
    ray.origin = origin;
    ray.direction = direction;
    ray.invDirection = 1.0 / direction;
    return ray;
}

bool intersectsBox(vec3 boundMin, vec3 boundMax, Ray ray, float maxT, out float enterT){
    vec3 tn = (boundMin - ray.origin) * ray.invDirection;
    vec3 tp = (boundMax - ray.origin) * ray.invDirection;

    vec3 ts = min(tn, tp);
    vec3 tb = max(tn, tp);

    float tmin = max(max(ts.x, ts.y), ts.z);
    float tmax = min(min(tb.x, tb.y), tb.z);

    enterT = max(tmin, 0.0);

    return tmax >= enterT && enterT <= maxT;
}

struct CandidateCursor{
    uint stack[MAX_TRAVERSAL_STACK];
    int stackSize;
    uint nextIndex;
    uint leafEnd;
    uint indexOffset;
    uint nodeOffset;
};

CandidateCursor beginCandidates(uint indexOffset, uint nodeOffset){
    CandidateCursor cursor;
    cursor.stack[0] = 0u; // Root.
    cursor.stackSize = 1;
    cursor.nextIndex = 0u;
    cursor.leafEnd = 0u;
    cursor.indexOffset = indexOffset;
    cursor.nodeOffset = nodeOffset;
    return cursor;
}

bool nextCandidate(inout CandidateCursor cursor, Ray ray, float maxT, out uint triangleIndex){
    triangleIndex = 0u;

    while(true){
        if(cursor.nextIndex < cursor.leafEnd){
            triangleIndex = cursor.nextIndex;
            cursor.nextIndex += 3u;

            return true;
        }

        if(cursor.stackSize == 0){
            return false;
        }

        cursor.stackSize--;
        BVHNode node = bvhNodes[cursor.nodeOffset + cursor.stack[cursor.stackSize]];

        float enterT;
        if(!intersectsBox(node.boundMin, node.boundMax, ray, maxT, enterT)){
            continue;
        }

        if(node.count > 0u){
            cursor.nextIndex = cursor.indexOffset + (node.leftOrStart * 3u);
            cursor.leafEnd = cursor.nextIndex + (node.count * 3u);
        }else{
            if(cursor.stackSize + 2 <= MAX_TRAVERSAL_STACK){
                cursor.stack[cursor.stackSize++] = node.leftOrStart + 1u;
                cursor.stack[cursor.stackSize++] = node.leftOrStart;
            }
        }
    }
}

vec4 castRay(vec2 coord, vec4 startColor, vec3 startPosition, vec3 startNormal){
    const float EPSILON = 0.00001;
    vec2 pos = coord / vec2(windowResolution);

    vec3 currentOrigin = startPosition;
    vec3 currentSurfaceNormal = normalize(startNormal);
    vec3 lastDirection = normalize(startPosition - camera.position.xyz);

    vec4 result = startColor;

    for(int bounce = 0; bounce < MAX_REFLECTIONS; bounce++){
        vec3 currentDirection = normalize(reflect(lastDirection, currentSurfaceNormal));

        float minT = -1.0f;
        int sphereIndex = -1;
        int triangleIndex = -1;
        vec3 triangleNormal = vec3(0.0);

        Ray worldRay = createRay(currentOrigin, currentDirection);

        for(uint i = 0u; i < instanceCount; i++){
            InstanceData instance = instances[i];
            uint vertexOffset = instance.geometry.x;
            uint triangleOffset = instance.geometry.y;
            uint triangleCount = instance.geometry.z;
            uint nodeOffset = instance.geometry.w;

            if(triangleCount == 0u){
                continue;
            }

            float enterT;
            if(!intersectsBox(vec3(instance.boundMin), vec3(instance.boundMax), worldRay, minT < 0.0 ? FLOAT_MAX : minT, enterT)){
                continue;
            }

            Ray localRay = createRay(
                vec3(instance.inverseModelMatrix * vec4(currentOrigin, 1.0)),
                vec3(instance.inverseModelMatrix * vec4(currentDirection, 0.0))
            );

            CandidateCursor cursor = beginCandidates(triangleOffset, nodeOffset);

            uint can;
            while(nextCandidate(cursor, localRay, minT < 0.0 ? FLOAT_MAX : minT, can)){
                vec3 p0 = vec3(vertices[vertexOffset + indices[can]].position);
                vec3 p1 = vec3(vertices[vertexOffset + indices[can + 1u]].position);
                vec3 p2 = vec3(vertices[vertexOffset + indices[can + 2u]].position);

                vec3 v0v1 = p1 - p0;
                vec3 v0v2 = p2 - p0;

                vec3 pv = cross(localRay.direction, v0v2);
                float det = dot(v0v1, pv);
                if(abs(det) < EPSILON){
                    continue;
                }

                float invDet = 1.0 / det;
                vec3 tv = localRay.origin - p0;
                float u = dot(tv, pv) * invDet;
                if(u < 0.0 || u > 1.0){
                    continue;
                }

                vec3 qv = cross(tv, v0v1);
                float v = dot(localRay.direction, qv) * invDet;
                if(v < 0.0 || (u + v) > 1.0){
                    continue;
                }

                float t = dot(v0v2, qv) * invDet;
                if(t < EPSILON){
                    continue;
                }

                if(t < minT || minT < 0.0f){
                    minT = t;
                    triangleIndex = int(i);

                    mat3 normalMatrix = mat3(
                        instance.normalMatrix[0].xyz,
                        instance.normalMatrix[1].xyz,
                        instance.normalMatrix[2].xyz
                    );
                    vec3 norm = normalize(normalMatrix * cross(v0v1, v0v2));
                    triangleNormal = det < 0.0 ? -norm : norm;
                }
            }
        }

        for(int i = 0; i < spheres.length(); i++){
            GPUSphere sphere = spheres[i];
            float radius = sphere.diameter * 0.5;
            vec3 L = sphere.position - currentOrigin;
            float tc = dot(L, currentDirection);
            if(tc < 0.0){
                continue;
            }

            float d = sqrt(dot(L, L) - (tc * tc));
            if(d < 0.0 || d > radius){
                continue;
            }
            float t1c = sqrt((radius * radius) - (d * d));
            float t1 = tc - t1c;
            float t2 = tc + t1c;

            if(t1 < EPSILON){
                // Assume is within self.
                continue;
            }

            if(t1 < minT || minT < 0.0f){
                minT = t1;
                sphereIndex = i;
                triangleIndex = -1; // Override triangles.
            }
        }

        currentOrigin = currentOrigin + (currentDirection * minT);
        if(triangleIndex >= 0){
            currentSurfaceNormal = triangleNormal;

            result.rgb *= vec3(0.9); // For testing.
        }else if(sphereIndex >= 0){
            GPUSphere sphere = spheres[sphereIndex];

            vec3 hitNormal = normalize(currentOrigin - sphere.position);
            currentSurfaceNormal = hitNormal;

            result.rgb *= sphere.baseColor.rgb; // TODO change to actual color.
        }else{
            break;
        }

        lastDirection = currentDirection;
    }

    return result;
}