#version 440

layout(location = 0) in vec2 translate;
layout(location = 1) in vec2 size;

out gl_PerVertex { vec4 gl_Position; };

layout(std140, binding = 0, set = 1) uniform buf {
    mat4 matrix;
    vec2 texsize;
    float layer;
} ub;

vec2 positions[4] = vec2[](
    vec2(0, 0),
    vec2(0, 1),
    vec2(1, 0),
    vec2(1, 1)
);

void main() {
    mat4 t = mat4(1, 0, 0, 0,
                  0, 1, 0, 0,
                  0, 0, 1, 0,
                  translate.x, translate.y, ub.layer, 1);
    gl_Position = ub.matrix * t * vec4(positions[gl_VertexIndex] * size, 0.0, 1.0);
}
