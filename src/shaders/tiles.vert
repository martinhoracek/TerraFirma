#version 440

layout(location = 0) in vec2 translate;
layout(location = 1) in vec2 size;
layout(location = 2) in vec2 uv;
layout(location = 3) in int paint;
layout(location = 4) in int slope;
layout(location = 0) out vec2 fraguv;
layout(location = 1) out int fragpaint;

out gl_PerVertex { vec4 gl_Position; };

layout(std140, binding = 0, set = 1) uniform buf {
    mat4 matrix;
    vec2 texsize;
    float layer;
} ub;

vec2 positions[8][4] = vec2[][](
    vec2[](  // slope 0
        vec2(0, 0),
        vec2(0, 1),
        vec2(1, 0),
        vec2(1, 1)
    ),
    vec2[](  // slope 1
        vec2(0, 0),
        vec2(0, 1),
        vec2(1, 1),
        vec2(1, 1)  // kill this point
    ),
    vec2[](  // slope 2
        vec2(0, 1),
        vec2(1, 1),
        vec2(1, 0),
        vec2(1, 0)  // kill this point
    ),
    vec2[](  // slope 3
        vec2(0, 0),
        vec2(0, 1),
        vec2(1, 0),
        vec2(1, 0)  // kill this point
    ),
    vec2[](  // slope 4
        vec2(0, 0),
        vec2(1, 1),
        vec2(1, 0),
        vec2(1, 0)  // kill this point
    ),
    vec2[](  // flip h
        vec2(0, 0),
        vec2(0, 1),
        vec2(1, 0),
        vec2(1, 1)
    ),
    vec2[](  // flip v
        vec2(0, 0),
        vec2(0, 1),
        vec2(1, 0),
        vec2(1, 1)
    ),
    vec2[](  // flip hv
        vec2(0, 0),
        vec2(0, 1),
        vec2(1, 0),
        vec2(1, 1)
    )
);

vec2 uvadds[8][4] = vec2[][](
    vec2[](  // slope 0
        vec2(0, 0),
        vec2(0, 1),
        vec2(1, 0),
        vec2(1, 1)
    ),
    vec2[](  // slope 1
        vec2(0, 0),
        vec2(0, 1),
        vec2(1, 0),
        vec2(1, 0)  // kill this point
    ),
    vec2[](  // slope 2
        vec2(0, 0),
        vec2(1, 1),
        vec2(1, 0),
        vec2(1, 0)  // kill this point
    ),
    vec2[](  // slope 3
        vec2(0, 0),
        vec2(0, 1),
        vec2(1, 1),
        vec2(1, 1)  // kill this point
    ),
    vec2[](  // slope 4
        vec2(0, 1),
        vec2(1, 1),
        vec2(1, 0),
        vec2(1, 0)  // kill this point
    ),
    vec2[](  // flip h
        vec2(1, 0),
        vec2(1, 1),
        vec2(0, 0),
        vec2(0, 1)
    ),
    vec2[](  // flip v
        vec2(0, 1),
        vec2(0, 0),
        vec2(1, 1),
        vec2(1, 0)
    ),
    vec2[](  // flip hv
        vec2(1, 1),
        vec2(1, 0),
        vec2(0, 1),
        vec2(0, 0)
    )
);

void main() {
    mat4 t = mat4(1, 0, 0, 0,
                  0, 1, 0, 0,
                  0, 0, 1, 0,
                  translate.x, translate.y, ub.layer, 1);
    vec2 dims = vec2((size.x - 0.5) / ub.texsize.x, (size.y - 0.5) / ub.texsize.y);
    
    gl_Position = ub.matrix * t * vec4(positions[slope][gl_VertexIndex] * size, 0.0, 1.0);
    fraguv = uv + uvadds[slope][gl_VertexIndex] * dims;
    fragpaint = paint;
}
