#version 430 core

in vec3 fragNormal;
in vec2 fragTextureUV;

out vec4 FragColor;

layout (binding = 0) uniform sampler2D colorTexture;
layout (location = 1) uniform vec4 baseColor;
layout (location = 2) uniform bool hasTexture;

void main(){
    vec4 color = baseColor;

    if(hasTexture){
        color *= texture(colorTexture, fragTextureUV);
    }

    float shade = max(dot(normalize(fragNormal), normalize(vec3(1.0, 0.5, 2.5))), 0.0);
    vec3 shaded = vec3(color) * ((shade + 1.0) / 2.0);
    FragColor = vec4(shaded, color.a);
}