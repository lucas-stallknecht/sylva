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

    GrassBlade blade;
    blade.position = vec3(
            push.chunk_world_origin.x + float(gx) * push.blade_step,
            push.chunk_world_origin.y,
            push.chunk_world_origin.z + float(gy) * push.blade_step
        );

    deref_i(push.attachments.blades, idx) = blade;
}
#endif
