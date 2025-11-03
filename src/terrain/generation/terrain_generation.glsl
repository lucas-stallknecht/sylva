#include <daxa/daxa.inl>

#extension GL_EXT_debug_printf : enable

#include "terrain_generation.inl"
#include "shader_shared/random.glsl"
#include "shader_shared/noise.glsl"

DAXA_DECL_PUSH_CONSTANT(GenerateTerrainPush, push)

#if DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_COMPUTE

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    if (texelCoord.x >= 4096 || texelCoord.y >= 4096) return;

    vec2 uv = vec2(texelCoord.xy) / (gl_NumWorkGroups.xy * gl_WorkGroupSize.xy);
    float height = 0.0;
    float amplitude = push.generation_params.amplitude;
    float frequency = 1.0;

    for (int i = 0; i < push.generation_params.octaves; i++) {
        height += amplitude * perlin_noise(uv * frequency / push.generation_params.scale);
        frequency *= push.generation_params.lacunarity;
        amplitude *= push.generation_params.persistence;
    }
    // vec4 color = vec4(vec3(height), 1.0);
    vec4 color = vec4(1.0);

    imageStore(daxa_image2D(push.attachments.terrain_albedo_map), texelCoord, color);
    imageStore(daxa_image2D(push.attachments.terrain_height_map), texelCoord, vec4(height, 0.0, 0.0, 1.0));
}

#endif
