#pragma once

#define PI 3.14159265

uint pcg_hash(uint seed)
{
    uint state = seed * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

float random_float(uint seed)
{
    return float(pcg_hash(seed)) * (1.0 / 4294967295.0);
}

float random_float(ivec2 v)
{
    uint seed = v.x * 1664525u + v.y * 1013904223u;
    return random_float(seed);
}

vec2 random_gradient(ivec2 v)
{
    float angle = random_float(v) * 2.0 * PI;
    return vec2(cos(angle), sin(angle));
}

vec2 random_vec2(ivec2 v)
{
    ivec2 seedX = ivec2(
            v.x * 127 + v.y * 74,
            v.y * 181 + v.x * 239
        );
    ivec2 seedY = ivec2(
            v.x * 331 + v.y * 97,
            v.y * 199 + v.x * 521
        );
    float x = random_float(seedX);
    float y = random_float(seedY);
    return vec2(x, y);
}

vec3 random_color(uint seed)
{
    float r = float(pcg_hash(seed * 0xA511E9B3u)) * (1.0 / 4294967295.0);
    float g = float(pcg_hash(seed * 0x63D837CBu)) * (1.0 / 4294967295.0);
    float b = float(pcg_hash(seed * 0xF1BBCDCBu)) * (1.0 / 4294967295.0);

    return vec3(r, g, b);
}
