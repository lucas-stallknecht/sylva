#pragma once

#include <daxa/daxa.hpp>
#include <daxa/utils/task_graph_types.hpp>
#include <daxa/pipeline.hpp>
#include <daxa/utils/pipeline_manager.hpp>

#include "grass_generation.inl"
#include "../common.h"

namespace sylva
{
    struct GrassGenerationParams
    {
        std::shared_ptr<std::vector<GrassChunk>> grass_chunks;
        float blade_step;
        std::uint32_t blades_per_side;
        float terrain_total_width;
    };

    inline daxa::ComputePipelineCompileInfo2 generate_grass_pipeline_compile_info()
    {
        return {
            .source = daxa::ShaderFile{"generation/grass_generation.glsl"},
            .push_constant_size = sizeof(GenerateGrassBladesPush),
        };
    };

    inline void generate_grass_callback(daxa::TaskInterface ti,
                                        std::shared_ptr<daxa::ComputePipeline> const & pipeline,
                                        GrassGenerationParams gen_params)
    {
        ti.recorder.set_pipeline(*pipeline);

        for (auto & chunk : *gen_params.grass_chunks)
        {
            GenerateGrassBladesPush chunk_push = {
                .chunk_world_origin = std::bit_cast<daxa_f32vec3>(chunk.world_origin),
                .chunk_seed = chunk.seed,
                .blade_step = gen_params.blade_step,
                .blades_per_side = gen_params.blades_per_side,
                .terrain_total_width = gen_params.terrain_total_width,
                .blade_buffer = ti.device.device_address(chunk.blade_buffer_id).value(),
                .attachments = ti.attachment_shader_blob,
            };

            ti.recorder.push_constant(chunk_push);

            std::uint32_t local_x = 8u;
            std::uint32_t local_y = 8u;
            std::uint32_t dispatch_x = (chunk_push.blades_per_side + local_x - 1) / local_x;
            std::uint32_t dispatch_y = (chunk_push.blades_per_side + local_y - 1) / local_y;

            ti.recorder.dispatch({
                .x = dispatch_x,
                .y = dispatch_y,
                .z = 1u,
            });
        }
    }

}; // namespace sylva
