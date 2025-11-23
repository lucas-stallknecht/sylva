#include <daxa/daxa.inl>

#extension GL_EXT_debug_printf : enable

#include "grass_generation.inl"
#include "shader_shared/random.glsl"
#include "shader_shared/noise.glsl"

DAXA_DECL_PUSH_CONSTANT(GenerateGrassBladesPush, push)

#if DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_COMPUTE

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

void main() {
    uint gx = gl_GlobalInvocationID.x;
    uint gy = gl_GlobalInvocationID.y;
    uint idx = gy * push.blades_per_side + gx;

    if (gx >= push.blades_per_side || gy >= push.blades_per_side) return;

    float world_x = push.chunk_world_origin.x + float(gx) * push.blade_step;
    float world_z = push.chunk_world_origin.z + float(gy) * push.blade_step;

    // Convert world-space position to terrain UV (0..1)
    float half_width = 0.5 * push.terrain_total_width;
    vec2 uv = clamp(vec2(
                (world_x + half_width) / push.terrain_total_width,
                (world_z + half_width) / push.terrain_total_width
            ), vec2(0.0), vec2(1.0));

    // === Corrected texel coordinate calculation ===
    daxa_ImageViewId height_img = push.attachments.terrain_height_map;
    ivec2 img_size = imageSize(daxa_image2D(height_img));
    vec2 img_size_f = vec2(img_size);

    // map uv to [0 .. size-1] instead of [0 .. size]
    ivec2 tex_coord = ivec2(
            clamp(floor(uv * (img_size_f - vec2(1.0))), vec2(0.0), img_size_f - vec2(1.0))
        );

    vec4 texel_val = imageLoad(daxa_image2D(height_img), tex_coord);
    float height = texel_val.r;

    GrassBlade blade;
    blade.position = vec3(
            world_x,
            height,
            world_z
        );
    blade.color = vec3(uv, 0.0);

    deref_i(push.blade_buffer, idx) = blade;
}
#endif
