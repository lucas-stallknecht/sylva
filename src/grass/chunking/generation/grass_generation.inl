#pragma once

#include <daxa/daxa.inl>
#include <daxa/utils/task_graph.inl>

struct GrassBlade
{
    daxa_f32vec3 position;
};
DAXA_DECL_BUFFER_PTR(GrassBlade)

DAXA_DECL_COMPUTE_TASK_HEAD_BEGIN(GenerateGrassBladesH)
DAXA_TH_IMAGE_ID(COMPUTE_SHADER::READ, REGULAR_2D, terrain_height_map)
DAXA_TH_IMAGE_ID(COMPUTE_SHADER::READ, REGULAR_2D, terrain_normal_map)
DAXA_TH_BUFFER_PTR(COMPUTE_SHADER::WRITE, daxa_BufferPtr(GrassBlade), blades)
DAXA_DECL_TASK_HEAD_END

struct GenerateGrassBladesPush
{
    daxa_f32 terrain_total_width;
    daxa_f32 blade_step;
    DAXA_TH_BLOB(GenerateGrassBladesH, attachments)
};
