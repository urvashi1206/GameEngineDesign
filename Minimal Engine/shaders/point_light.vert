#version 450

const vec2 OFFSETS[6] = vec2[](
vec2(-1.0, -1.0),
vec2(-1.0, 1.0),
vec2(1.0, -1.0),
vec2(1.0, -1.0),
vec2(-1.0, 1.0),
vec2(1.0, 1.0)
);

layout(location = 0) out vec2 fragOffset;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    vec4 ambientColor;// w is light intensity
    vec3 lightPosition;
    vec4 lightColor;// w is light intensity
} ubo;

const float LIGHT_RADIUS = 0.05;

void main() {
    fragOffset = OFFSETS[gl_VertexIndex];
    vec3 cameraWorldRight = { ubo.view[0][0], ubo.view[1][0], ubo.view[2][0] };
    vec3 cameraWorldUp = { ubo.view[0][1], ubo.view[1][1], ubo.view[2][1] };

    vec3 worldPostion = ubo.lightPosition.xyz
    + LIGHT_RADIUS * fragOffset.x * cameraWorldRight
    + LIGHT_RADIUS * fragOffset.y * cameraWorldUp;

    gl_Position = ubo.projection * ubo.view * vec4(worldPostion, 1.0);
}
