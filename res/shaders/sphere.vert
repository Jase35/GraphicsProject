layout (location = 0) uniform uint instanceOffset;

out vec2 fragOffset;
flat out uint sphereIndex;

void main(){
    sphereIndex = instanceOffset + uint(gl_InstanceID);
    GPUSphere sphere = spheres[sphereIndex];

    vec2 corner = vec2(
        (gl_VertexID & 1) == 0 ? -1.0 : 1.0,
        (gl_VertexID & 2) == 0 ? -1.0 : 1.0
    );

    vec3 viewCenter = (camera.view * vec4(sphere.position, 1.0)).xyz;
    vec3 viewPosition = viewCenter + vec3(corner * (0.5 * sphere.diameter), 0.0);

    fragOffset = corner;
    gl_Position = camera.projection * vec4(viewPosition, 1.0);
}