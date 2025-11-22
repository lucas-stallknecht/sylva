#pragma once

#include <daxa/daxa.inl>
#include <daxa/utils/task_graph.inl>

#include "../shader_shared/shared.inl"

DAXA_DECL_RASTER_TASK_HEAD_BEGIN(DrawOpaqueH)
DAXA_TH_BUFFER_PTR(RASTER_SHADER::READ, daxa_BufferPtr(CamInfo), camera)
DAXA_TH_BUFFER_PTR(VERTEX_SHADER::READ, daxa_BufferPtr(Vertex), vertices)
DAXA_TH_IMAGE_ID(RASTER_SHADER::READ, REGULAR_2D, terrain_height_map)
DAXA_TH_IMAGE_ID(FRAGMENT_SHADER::READ, REGULAR_2D, terrain_albedo_map)
DAXA_TH_IMAGE_ID(FRAGMENT_SHADER::READ, REGULAR_2D, terrain_normal_map)
DAXA_TH_IMAGE(COLOR_ATTACHMENT, REGULAR_2D, dst_img)
DAXA_TH_IMAGE(DEPTH_ATTACHMENT, REGULAR_2D, depth_img)
DAXA_DECL_TASK_HEAD_END

struct DrawTerrainPush
{
    daxa_SamplerId linear_sampler;
    DAXA_TH_BLOB(DrawOpaqueH, attachments)
};

struct DrawGrassBladesPush
{
    daxa_SamplerId linear_sampler;
    DAXA_TH_BLOB(DrawOpaqueH, attachments)
};
