#version 440

layout(set = 2, binding = 0) uniform sampler2D tex;
layout(location = 0) in vec2 uv;
layout(location = 1) flat in int paint;

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0, set = 3) uniform buf {
    bool hiliting;
} ub;

void main() {
    vec4 c = texture(tex, uv);

    int paintgroup = paint;
    // only paint grass part of a grass block
    if (paintgroup > 27 && paintgroup < 40) {
        if (c.b * 0.5 < c.g && c.g * 0.5 < c.b && c.r * 0.3 < c.b &&
            c.r * 0.8 > c.b && c.r * 0.8 > c.g && c.r * 0.3 < c.g) {
            paintgroup = 0;
        } else {
            paintgroup -= 27;
        }
    }

    if (paintgroup > 0) {
        float hi = max(c.r, max(c.g, c.b));
        float lo = min(c.r, min(c.g, c.b));
        float origLo = lo;
        if (paintgroup > 12) {
            if (paintgroup < 25) {
                lo *= 0.4;
            }
            paintgroup -= 12;
        }
        float md = (hi + lo) / 2.0;
        switch (paintgroup) {
            case 1: c.rgb = vec3(hi, lo, lo); break;
            case 2: c.rgb = vec3(hi, md, lo); break;
            case 3: c.rgb = vec3(hi, hi, lo); break;
            case 4: c.rgb = vec3(md, hi, lo); break;
            case 5: c.rgb = vec3(lo, hi, lo); break;
            case 6: c.rgb = vec3(lo, hi, md); break;
            case 7: c.rgb = vec3(lo, hi, hi); break;
            case 8: c.rgb = vec3(lo, md, hi); break;
            case 9: c.rgb = vec3(lo, lo, hi); break;
            case 10: c.rgb = vec3(md, lo, hi); break;
            case 11: c.rgb = vec3(hi, lo, hi); break;
            case 12: c.rgb = vec3(hi, lo, md); break;
            case 13:
                float black = (hi + lo) * 0.15;
                c.rgb = vec3(black, black, black);
                break;
            case 14:
                float intensity = (hi * 0.7 + lo * 0.3) * (2 - (hi + lo) / 2.0);
                c.rgb = vec3(intensity, intensity, intensity);
                break;
            case 15: c.rgb = vec3(md, md, md); break;
            case 28: c.rgb = vec3(hi, hi * 0.7, hi * 0.49); break;
            case 29:
                float dark = (hi + lo) * 0.025;
                c.rgb = vec3(dark, dark, dark);
                break;
            case 30:
                if (hi > 0) {
                    c.rgb = vec3(1.0 - c.r, 1.0 - c.g, 1.0 - c.b);
                }
                break;
            case 31:
                if (hi > 0) {
                    c.r = max(0.75 - c.r * 2, 0);
                    c.g = max(0.75 - c.g * 2, 0);
                    c.b = max(0.75 - c.b * 2, 0);
                } else {
                    c.r *= 2;
                    c.g *= 2;
                    c.b *= 2;
                }
                break;
        }
    }

    if (c.a < 0.1) {
        discard;
    }
    if (ub.hiliting) {
        c.rgb *= vec3(0.3, 0.3, 0.3);
    }
    c.rgb = pow(c.rgb, vec3(2.2));  // gamma
    fragColor = c;
}
