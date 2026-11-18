sample in vec2 fragOffset;
flat in uint sphereIndex;

out vec4 FragColor;

void main(){
    if(dot(fragOffset, fragOffset) > 1.0){
        discard;
    }

    vec3 viewNormal = vec3(fragOffset, 
        sqrt(1.0 - (fragOffset.x * fragOffset.x) - (fragOffset.y * fragOffset.y))
    );

    vec3 worldNormal = normalize(transpose(mat3(camera.view)) * viewNormal);

    GPUSphere sphere = spheres[sphereIndex];

    vec2 uv = sphereUV(worldNormal);
    vec4 color = materialColor(sphere.materialIndex, uv, dFdx(uv), dFdy(uv));

    vec3 worldPosition = sphere.position + (worldNormal * sphere.diameter * 0.5);

    FragColor = castRay(gl_FragCoord.xy, color, worldPosition, worldNormal);
}