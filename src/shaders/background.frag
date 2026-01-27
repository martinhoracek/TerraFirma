#version 440

layout(set = 2, binding = 0) uniform sampler2D tex;
layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0, set = 3) uniform buf {
    bool hiliting;
} ub;

void main() {
    vec4 c = texture(tex, uv);
    if (ub.hiliting) {
        c.rgb *= vec3(0.3, 0.3, 0.3);
    }
    c.rgb = pow(c.rgb, vec3(2.2));  // gamma
    fragColor = c;
}
