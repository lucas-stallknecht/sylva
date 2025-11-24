vec3 bezier(float t, vec3 c0, vec3 c1, vec3 c2)
{
    return c2
        + pow(1.0 - t, 2.0) * (c0 - c2)
        + pow(t, 2.0) * (c1 - c2);
}

vec3 dBezier(float t, vec3 c0, vec3 c1, vec3 c2)
{
    return 2.0 * (1.0 - t) * (c0 - c2)
        - 2.0 * t * (c1 - c2);
}
