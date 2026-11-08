layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textureUV;
layout (location = 3) in ivec4 boneIndices;
layout (location = 4) in vec4 boneWeights;

sample out vec3 fragPosition;
sample out vec3 fragNormal;
sample out vec2 fragTextureUV;

layout (std430, binding = 2) readonly buffer BoneBuffer{
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

    vec4 worldPosition = globalModel * vec4(position, 1.0);

    gl_Position = camera.projection * camera.view * worldPosition;
    fragPosition = worldPosition.xyz / worldPosition.w;

    fragTextureUV = textureUV;
}