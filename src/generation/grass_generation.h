#pragma once

#include <daxa/daxa.hpp>
#include <daxa/utils/task_graph_types.hpp>
#include <daxa/pipeline.hpp>
#include <daxa/utils/pipeline_manager.hpp>

#include "grass_generation.inl"

namespace sylva
{

    inline daxa::ComputePipelineCompileInfo2 generate_grass_pipeline_compile_info()
    {
        return {
            .source = daxa::ShaderFile{"generation/grass_generation.glsl"},
            .push_constant_size = sizeof(GenerateGrassBladesPush),
        };
    };

    inline void generate_grass_callback(daxa::TaskInterface ti,
                                        std::shared_ptr<daxa::ComputePipeline> const & pipeline,
                                        GenerateGrassBladesPush push_constants)
    {
        ti.recorder.set_pipeline(*pipeline);

        push_constants.attachments = ti.attachment_shader_blob;
        ti.recorder.push_constant(push_constants);

        std::uint32_t local_x = 8u;
        std::uint32_t local_y = 8u;
        std::uint32_t dispatch_x = (push_constants.blades_per_side + local_x - 1) / local_x;
        std::uint32_t dispatch_y = (push_constants.blades_per_side + local_y - 1) / local_y;

        ti.recorder.dispatch({
            .x = dispatch_x,
            .y = dispatch_y,
            .z = 1u,
        });
    }

}; // namespace sylva
