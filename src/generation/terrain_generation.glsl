#include <daxa/daxa.inl>

#extension GL_EXT_debug_printf : enable

#include "terrain_generation.inl"
#include "shader_shared/random.glsl"
#include "shader_shared/noise.glsl"

DAXA_DECL_PUSH_CONSTANT(GenerateTerrainPush, push)

#if DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_COMPUTE

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

float compute_height(vec2 uv) {
    float height = 0.0;
    float amplitude = push.generation_params.amplitude;
    float frequency = 1.0;

    for (int i = 0; i < push.generation_params.octaves; i++) {
        height += amplitude * (2.0 * perlin_noise(uv * frequency / push.generation_params.scale) - 1.0);
        frequency *= push.generation_params.lacunarity;
        amplitude *= push.generation_params.persistence;
    }

    return height;
}

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    if (texelCoord.x >= 4096 || texelCoord.y >= 4096) return;

    vec2 uv = vec2(texelCoord.xy) / (gl_NumWorkGroups.xy * gl_WorkGroupSize.xy);
    float height = compute_height(uv);

    vec2 eps = vec2(0.0001, 0.0);

    float hL = compute_height(uv - eps.xy);
    float hR = compute_height(uv + eps.xy);
    float hD = compute_height(uv - eps.yx);
    float hU = compute_height(uv + eps.yx);

    float scl = push.generation_params.scale;
    vec3 normal = normalize(vec3(
                (hL - hR),
                (2.0 * eps.x / scl),
                (hD - hU)
            ));

    // Normalize vertical scaling
    float amp = push.generation_params.amplitude;
    float dX = (hL - hR) / (amp);
    float dY = (hD - hU) / (amp);

    vec3 color_normal = normalize(vec3(
                dX,
                // Compensate for UV scaling stretching the terrain
                (2.0 * eps.x / scl),
                dY
            ));
    float slope = 1.0 - color_normal.y;
    float t = smoothstep(push.generation_params.slope_min, push.generation_params.slope_max, slope);

    vec3 grass_color = vec3(0.259, 0.439, 0.184);
    vec3 rock_color = vec3(0.190, 0.178, 0.151);

    vec3 color = vec3(mix(grass_color, rock_color, t));

    imageStore(daxa_image2D(push.attachments.terrain_albedo_map), texelCoord, vec4(color, 1.0));
    imageStore(daxa_image2D(push.attachments.terrain_normal_map), texelCoord, vec4(0.5 * normal + 0.5, 1.0));
    imageStore(daxa_image2D(push.attachments.terrain_height_map), texelCoord, vec4(height, 0.0, 0.0, 1.0));
}

#endif
