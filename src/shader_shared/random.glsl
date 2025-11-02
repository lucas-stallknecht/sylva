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
