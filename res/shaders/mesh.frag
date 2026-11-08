sample in vec3 fragPosition;
sample in vec3 fragNormal;
sample in vec2 fragTextureUV;

layout (binding = 1) uniform sampler2D colorTexture;
layout (location = 2) uniform vec4 baseColor;
layout (location = 3) uniform bool hasTexture;

out vec4 FragColor;

void main(){
    vec4 color = baseColor;

    if(hasTexture){
        color *= texture(colorTexture, fragTextureUV);
    }

    FragColor = castRay(gl_FragCoord.xy, color, fragPosition, fragNormal);
}