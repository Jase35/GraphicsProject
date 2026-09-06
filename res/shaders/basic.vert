#version 430 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textureUV;
layout (location = 3) in ivec4 boneIndices;
layout (location = 4) in vec4 boneWeights;

out vec3 fragNormal;
out vec2 fragTextureUV;

layout (std140, binding = 0) uniform CameraBuffer{
    mat4 view;
    mat4 projection;
}camera;

struct InstanceData{
    mat4 modelMatrix;
    vec4 normalMatrix[3];
    vec4 padding;
};

layout (std430, binding = 0) readonly buffer InstanceBuffer{
    InstanceData instances[];
};

layout (std430, binding = 1) readonly buffer BoneBuffer{
    mat4 matrices[];
}bones;

layout (location = 0) uniform uint instanceOffset;

void main(){
    mat4 globalModel;
    uint instance = instanceOffset + uint(gl_InstanceID);

    if(boneWeights.x + boneWeights.y + boneWeights.z + boneWeights.w > 0.001){
        globalModel = (
            boneWeights.x * bones.matrices[boneIndices.x] +
            boneWeights.y * bones.matrices[boneIndices.y] +
            boneWeights.z * bones.matrices[boneIndices.z] +
            boneWeights.w * bones.matrices[boneIndices.w]
        );
        fragNormal = normalize(mat3(globalModel) * normal);
    }else{
        globalModel = instances[instance].modelMatrix;
        fragNormal = normalize(mat3(
            instances[instance].normalMatrix[0].xyz,
            instances[instance].normalMatrix[1].xyz,
            instances[instance].normalMatrix[2].xyz
        ) * normal);
    }

    //shade = max(dot(globalNormal, normalize(vec3(1.0, 0.5, 2.5))), 0.0);
    gl_Position = camera.projection * camera.view * globalModel * vec4(position, 1.0);
    fragTextureUV = textureUV;
}