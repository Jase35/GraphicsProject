sample in vec3 fragPosition;
sample in vec3 fragNormal;
sample in vec2 fragTextureUV;
flat in uint fragInstance;

out vec4 FragColor;

void main(){
    vec4 color = materialColor(
        instances[fragInstance].material.x,
        fragTextureUV,
        dFdx(fragTextureUV),
        dFdy(fragTextureUV)
    );

    FragColor = castRay(gl_FragCoord.xy, color, fragPosition, fragNormal);
}