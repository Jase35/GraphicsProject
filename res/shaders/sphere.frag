sample in vec2 fragOffset;
flat in uint sphereIndex;

out vec4 FragColor;

layout (binding = 1) uniform sampler2D colorTexture;
layout (location = 2) uniform vec4 baseColor;
layout (location = 3) uniform bool hasTexture;

void main(){
    if(dot(fragOffset, fragOffset) > 1.0){
        discard;
    }

    vec3 viewNormal = vec3(fragOffset, 
        sqrt(1.0 - (fragOffset.x * fragOffset.x) - (fragOffset.y * fragOffset.y))
    );

    vec3 worldNormal = normalize(transpose(mat3(camera.view)) * viewNormal);

    vec4 color = baseColor;
    if(hasTexture){
        // TODO color *= ?
    }

    GPUSphere sphere = spheres[sphereIndex];
    vec3 worldPosition = sphere.position + (worldNormal * sphere.diameter * 0.5);

    FragColor = castRay(gl_FragCoord.xy, color, worldPosition, worldNormal);
}