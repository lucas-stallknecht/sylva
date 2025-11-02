#pragma once

#include <daxa/daxa.hpp>
#include <daxa/utils/task_graph_types.hpp>
#include <daxa/pipeline.hpp>

#include "daxa/utils/pipeline_manager.hpp"
#include "terrain_generation.inl"

namespace sylva
{
    inline daxa::ComputePipelineCompileInfo2 generate_terrain_pipeline_compile_info()
    {
        return {
            .source = daxa::ShaderFile{"terrain/generation/terrain_generation.glsl"},
            .push_constant_size = sizeof(GenerateTerrainPush),
        };
    }

    inline void generate_terrain_callback(daxa::TaskInterface ti, daxa::ComputePipeline * pipeline)
    {
        ti.recorder.set_pipeline(*pipeline);
        ti.recorder.push_constant(GenerateTerrainPush{
            .generation_params =
                {
                    .amplitude = 0.3f,
                    .scale = 0.5f,
                    .octaves = 4,
                    .persistence = 0.5f,
                    .lacunarity = 2.0f,
                },
            .attachments = ti.attachment_shader_blob,
        });
        ti.recorder.dispatch({
            .x = 4096 / 8,
            .y = 4096 / 8,
            .z = 1,
        });
    };

} // namespace sylva
