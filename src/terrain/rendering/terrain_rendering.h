#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <daxa/daxa.hpp>
#include <daxa/utils/pipeline_manager.hpp>
#include <cstddef>

#include "terrain_rendering.inl"

namespace sylva
{

    struct TerrainInfo
    {
        std::size_t patches = 10u;
        float width_x = 1.0f;
        float width_y = 1.0f;
    };

    struct RenderTerrainContext
    {
        daxa::TaskBuffer vertex_buffer;
        std::size_t vertex_count = 0;

        explicit RenderTerrainContext(daxa::Device & device) { update_from_terrain_info(device); };

        void update_from_terrain_info(daxa::Device & device, TerrainInfo const & terrain_info = {});
    };

    inline daxa::RasterPipelineCompileInfo2
    tesselate_terrain_pipeline_compile_info(daxa::Format format)
    {
        return {
            .vertex_shader_info =
                daxa::ShaderCompileInfo2{
                    .source = daxa::ShaderFile{"terrain/rendering/terrain_rendering.glsl"}},
            .tesselation_control_shader_info =
                daxa::ShaderCompileInfo2{
                    .source = daxa::ShaderFile{"terrain/rendering/terrain_rendering.glsl"}},
            .tesselation_evaluation_shader_info =
                daxa::ShaderCompileInfo2{
                    .source = daxa::ShaderFile{"terrain/rendering/terrain_rendering.glsl"}},
            .fragment_shader_info =
                daxa::ShaderCompileInfo2{
                    .source = daxa::ShaderFile{"terrain/rendering/terrain_rendering.glsl"}},
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
            .push_constant_size = sizeof(RenderTerrainPush),
        };
    };
    std::vector<TerrainVertex> generate_terrain_vertices(TerrainInfo const & terrain_info);
    void render_terrain_callback(daxa::TaskInterface ti,
                                 std::shared_ptr<daxa::RasterPipeline> const & pipeline,
                                 RenderTerrainContext * terrain_context, daxa::SamplerId sampler,
                                 daxa::TaskBuffer const & cam_buffer);

} // namespace sylva
