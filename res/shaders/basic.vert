#version 430 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textureUV;
layout (location = 3) in ivec4 boneIndices;
layout (location = 4) in vec4 boneWeights;

uniform mat4 view;
uniform mat4 projection;

uniform mat4 modelMatrix;
uniform mat3 normalMatrix;

out vec3 fragNormal;
out vec2 fragTextureUV;

layout (std430, binding = 0) readonly buffer BoneBuffer{
    mat4 matrices[];
}bones;

void main(){
    mat4 globalModel;

    if(boneWeights.x + boneWeights.y + boneWeights.z + boneWeights.w > 0.001){
        globalModel = (
            boneWeights.x * bones.matrices[boneIndices.x] +
            boneWeights.y * bones.matrices[boneIndices.y] +
            boneWeights.z * bones.matrices[boneIndices.z] +
            boneWeights.w * bones.matrices[boneIndices.w]
        );
        fragNormal = normalize(mat3(globalModel) * normal);
    }else{
        globalModel = modelMatrix;
        fragNormal = normalize(normalMatrix * normal);
    }

    //shade = max(dot(globalNormal, normalize(vec3(1.0, 0.5, 2.5))), 0.0);
    gl_Position = projection * view * globalModel * vec4(position, 1.0);
    fragTextureUV = textureUV;
}