#pragma once

#include <daxa/daxa.hpp>
#include <daxa/utils/pipeline_manager.hpp>

#include "opaque.inl"

namespace sylva
{
    inline daxa::RasterPipelineCompileInfo2
    tesselate_terrain_pipeline_compile_info(daxa::Format format)
    {
        return {
            .vertex_shader_info =
                daxa::ShaderCompileInfo2{.source =
                                             daxa::ShaderFile{"rendering/terrain_rendering.glsl"}},
            .tesselation_control_shader_info =
                daxa::ShaderCompileInfo2{.source =
                                             daxa::ShaderFile{"rendering/terrain_rendering.glsl"}},
            .tesselation_evaluation_shader_info =
                daxa::ShaderCompileInfo2{.source =
                                             daxa::ShaderFile{"rendering/terrain_rendering.glsl"}},
            .fragment_shader_info =
                daxa::ShaderCompileInfo2{.source =
                                             daxa::ShaderFile{"rendering/terrain_rendering.glsl"}},
            .color_attachments = {{.format = format}},
            .depth_test =
                daxa::DepthTestInfo{
                    .depth_attachment_format = daxa::Format::D32_SFLOAT,
                    .enable_depth_write = true,
                },
            .raster =
                {
                    .primitive_topology = daxa::PrimitiveTopology::PATCH_LIST,
                    // .polygon_mode = daxa::PolygonMode::LINE,
                    // .line_width = 0.01f,
                },
            .tesselation = {.control_points = 4},
            .push_constant_size = sizeof(DrawTerrainPush),
        };
    }

    inline daxa::RasterPipelineCompileInfo2
    draw_grass_blades_pipeline_compile_info(daxa::Format format)
    {
        return {
            .vertex_shader_info =
                daxa::ShaderCompileInfo2{.source =
                                             daxa::ShaderFile{"rendering/grass_rendering.glsl"}},
            .fragment_shader_info =
                daxa::ShaderCompileInfo2{.source =
                                             daxa::ShaderFile{"rendering/grass_rendering.glsl"}},
            .color_attachments = {{.format = format}},
            .depth_test =
                daxa::DepthTestInfo{
                    .depth_attachment_format = daxa::Format::D32_SFLOAT,
                    .enable_depth_write = true,
                },
            .push_constant_size = sizeof(DrawGrassBladesPush),
        };
    }
} // namespace sylva
