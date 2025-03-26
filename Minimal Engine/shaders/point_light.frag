#version 450

layout (location = 0) in vec2 fragOffset;
layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    vec4 ambientColor;// w is light intensity
    vec3 lightPosition;
    vec4 lightColor;// w is light intensity
} ubo;

void main() {
    float dis = sqrt(dot(fragOffset, fragOffset));
    if (dis >= 1.0f){
        discard;
    }
    outColor = vec4(ubo.lightColor.xyz, 1.0);
}
