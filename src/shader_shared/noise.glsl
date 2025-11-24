#pragma once

#include "random.glsl"

float value_noise(vec2 p)
{
    ivec2 i = ivec2(floor(p));
    vec2 f = fract(p);

    // corner values
    float v00 = random_float(i);
    float v10 = random_float(i + ivec2(1, 0));
    float v01 = random_float(i + ivec2(0, 1));
    float v11 = random_float(i + ivec2(1, 1));

    vec2 s = smoothstep(0.0, 1.0, f);

    float nx0 = mix(v00, v10, s.x);
    float nx1 = mix(v01, v11, s.x);
    return mix(nx0, nx1, s.y);
}

vec2 value_noise2(vec2 p) {
    ivec2 i = ivec2(floor(p));
    vec2 f = fract(p);

    vec2 v00 = random_vec2(i);
    vec2 v10 = random_vec2(i + ivec2(1, 0));
    vec2 v01 = random_vec2(i + ivec2(0, 1));
    vec2 v11 = random_vec2(i + ivec2(1, 1));

    vec2 s = smoothstep(0.0, 1.0, f);

    vec2 nx0 = mix(v00, v10, s.x);
    vec2 nx1 = mix(v01, v11, s.x);
    return mix(nx0, nx1, s.y);
}

float perlin_noise(vec2 p)
{
    ivec2 pi = ivec2(floor(p));
    vec2 pf = fract(p);

    // corners
    ivec2 p00 = pi;
    ivec2 p10 = pi + ivec2(1, 0);
    ivec2 p01 = pi + ivec2(0, 1);
    ivec2 p11 = pi + ivec2(1, 1);

    // gradient vectors
    vec2 g00 = random_gradient(p00);
    vec2 g10 = random_gradient(p10);
    vec2 g01 = random_gradient(p01);
    vec2 g11 = random_gradient(p11);

    // distance vectors
    vec2 d00 = pf - vec2(0.0, 0.0);
    vec2 d10 = pf - vec2(1.0, 0.0);
    vec2 d01 = pf - vec2(0.0, 1.0);
    vec2 d11 = pf - vec2(1.0, 1.0);

    // dot products
    float n00 = dot(g00, d00);
    float n10 = dot(g10, d10);
    float n01 = dot(g01, d01);
    float n11 = dot(g11, d11);

    // quintic fade curve (6t^5 − 15t^4 + 10t^3) for smoother transitions
    vec2 u = pf * pf * pf * (pf * (pf * 6.0 - 15.0) + 10.0);

    float nx0 = mix(n00, n10, u.x);
    float nx1 = mix(n01, n11, u.x);
    float nxy = mix(nx0, nx1, u.y);

    // Remap result to [0,1]
    return nxy * 0.5 + 0.5;
}
