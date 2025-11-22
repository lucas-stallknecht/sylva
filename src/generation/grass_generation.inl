#pragma once

#include <daxa/daxa.inl>
#include <daxa/utils/task_graph.inl>
#include "../shader_shared/shared.inl"

DAXA_DECL_COMPUTE_TASK_HEAD_BEGIN(GenerateGrassBladesH)
DAXA_TH_IMAGE_ID(COMPUTE_SHADER::READ, REGULAR_2D, terrain_height_map)
DAXA_TH_IMAGE_ID(COMPUTE_SHADER::READ, REGULAR_2D, terrain_normal_map)
DAXA_TH_BUFFER_PTR(COMPUTE_SHADER::WRITE, daxa_BufferPtr(GrassBlade), blades)
DAXA_DECL_TASK_HEAD_END

struct GenerateGrassBladesPush
{
    daxa_f32vec3 chunk_world_origin;
    daxa_u32 chunk_seed;
    daxa_f32 blade_step;
    daxa_u32 blades_per_side;
    daxa_f32 terrain_total_width;
    DAXA_TH_BLOB(GenerateGrassBladesH, attachments)
};
