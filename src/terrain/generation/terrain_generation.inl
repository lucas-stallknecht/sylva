#pragma once

#include <daxa/daxa.inl>
#include <daxa/utils/task_graph.inl>

DAXA_DECL_COMPUTE_TASK_HEAD_BEGIN(GenerateTerrainH)
DAXA_TH_IMAGE_ID(COMPUTE_SHADER::WRITE, REGULAR_2D, terrain_height_map)
DAXA_TH_IMAGE_ID(COMPUTE_SHADER::WRITE, REGULAR_2D, terrain_albedo_map)
DAXA_TH_IMAGE_ID(COMPUTE_SHADER::WRITE, REGULAR_2D, terrain_normal_map)
DAXA_DECL_TASK_HEAD_END

struct TerrainGenerationParams
{
    daxa_f32 amplitude;
    daxa_f32 scale;
    daxa_u32 octaves;
    daxa_f32 persistence;
    daxa_f32 lacunarity;
    daxa_f32 slope_min;
    daxa_f32 slope_max;
};

struct GenerateTerrainPush
{
    TerrainGenerationParams generation_params;
    DAXA_TH_BLOB(GenerateTerrainH, attachments)
};
