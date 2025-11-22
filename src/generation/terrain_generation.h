#pragma once

#include <daxa/daxa.hpp>
#include <daxa/utils/task_graph_types.hpp>
#include <daxa/pipeline.hpp>
#include <daxa/utils/pipeline_manager.hpp>

#include "terrain_generation.inl"
#include "../defaults.h"

namespace sylva
{
    inline daxa::ComputePipelineCompileInfo2 generate_terrain_pipeline_compile_info()
    {
        return {
            .source = daxa::ShaderFile{"generation/terrain_generation.glsl"},
            .push_constant_size = sizeof(GenerateTerrainPush),
        };
    }

    inline void generate_terrain_callback(daxa::TaskInterface ti,
                                          std::shared_ptr<daxa::ComputePipeline> const & pipeline,
                                          TerrainGenerationParams const * params)
    {
        ti.recorder.set_pipeline(*pipeline);
        ti.recorder.push_constant(GenerateTerrainPush{
            .generation_params = *params,
            .attachments = ti.attachment_shader_blob,
        });
        ti.recorder.dispatch({
            .x = defaults::terrain_map_resolution / 8u,
            .y = defaults::terrain_map_resolution / 8u,
            .z = 1u,
        });
    };

} // namespace sylva
