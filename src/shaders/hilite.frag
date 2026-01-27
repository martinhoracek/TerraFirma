#version 440

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0, set = 3) uniform buf {
    bool hiliting;
    float amount;
} ub;

void main() {
    fragColor = vec4(1.0, 0.8, 1.0, ub.amount);
}
