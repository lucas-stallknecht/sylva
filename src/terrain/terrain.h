#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <daxa/daxa.hpp>
#include <daxa/utils/pipeline_manager.hpp>

#include "terrain.inl"

namespace sylva
{

    struct TerrainInfo
    {
        unsigned int patches = 40u;
        float width_x = 1.0f;
        float width_y = 1.0f;
    };

    std::vector<TerrainVertex> generate_terrain_vertices(TerrainInfo const & terrain_info);

    inline daxa::RasterPipelineCompileInfo2
    tesselate_terrain_pipeline_compile_info(daxa::Format format)
    {
        return {
            .vertex_shader_info =
                daxa::ShaderCompileInfo2{.source = daxa::ShaderFile{"src/terrain/terrain.glsl"}},
            .tesselation_control_shader_info =
                daxa::ShaderCompileInfo2{.source = daxa::ShaderFile{"src/terrain/terrain.glsl"}},
            .tesselation_evaluation_shader_info =
                daxa::ShaderCompileInfo2{.source = daxa::ShaderFile{"src/terrain/terrain.glsl"}},
            .fragment_shader_info =
                daxa::ShaderCompileInfo2{.source = daxa::ShaderFile{"src/terrain/terrain.glsl"}},
            .color_attachments = {{.format = format}},
            .raster =
                {
                    .primitive_topology = daxa::PrimitiveTopology::PATCH_LIST,
                    .polygon_mode = daxa::PolygonMode::LINE,
                    .line_width = 0.01f,
                },
            .tesselation = {.control_points = 4},
            .push_constant_size = sizeof(TesselateTerrainPush),
        };
    };

    struct TerrainContext
    {
        daxa::TaskBuffer vertex_buffer = {};
        daxa::u32 vertex_count = 0;

        explicit TerrainContext(daxa::Device device) { update_from_terrain_info(device); };

        void update_from_terrain_info(daxa::Device device, TerrainInfo const & terrain_info = {});
    };

    struct TesselateTerrainTask : TesselateTerrainH::Task
    {
        AttachmentViews views = {};
        daxa::RasterPipeline * pipeline = nullptr;
        TerrainContext * terrain_context = nullptr;
        daxa::SamplerId sampler = {};

        void callback(daxa::TaskInterface ti);
    };
} // namespace sylva
