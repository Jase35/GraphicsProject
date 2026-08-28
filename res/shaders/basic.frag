#version 430 core

in vec3 fragNormal;
in vec2 fragTextureUV;

out vec4 FragColor;

void main(){
    float shade = max(dot(normalize(fragNormal), normalize(vec3(1.0, 0.5, 2.5))), 0.0);
    FragColor = vec4(vec3(0.0, 1.0, 0.0) * ((shade + 1.0) / 2.0), 1.0);
}