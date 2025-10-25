#pragma once

#include <daxa/daxa.inl>
#include <daxa/utils/task_graph.inl>

struct TerrainVertex
{
    daxa_f32vec3 position;
    daxa_f32vec2 uv;
};
DAXA_DECL_BUFFER_PTR(TerrainVertex)

DAXA_DECL_RASTER_TASK_HEAD_BEGIN(TesselateTerrainH)
DAXA_TH_BUFFER_PTR(VERTEX_SHADER::READ, daxa_BufferPtr(TerrainVertex), vertices)
DAXA_TH_IMAGE_ID(RASTER_SHADER::READ, REGULAR_2D, height_map)
DAXA_TH_IMAGE(COLOR_ATTACHMENT, REGULAR_2D, dst_img)
DAXA_DECL_TASK_HEAD_END

struct TesselateTerrainPush
{
    daxa_f32mat4x4 proj_view;
    DAXA_TH_BLOB(TesselateTerrainH, attachments)
};
