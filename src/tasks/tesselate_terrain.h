#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <daxa/daxa.hpp>
#include <daxa/utils/pipeline_manager.hpp>

#include "tesselate_terrain.inl"

namespace sylva
{
    inline daxa::RasterPipelineCompileInfo2
    tesselate_terrain_pipeline_compile_info(daxa::Format format)
    {
        return {
            .vertex_shader_info =
                daxa::ShaderCompileInfo2{.source =
                                             daxa::ShaderFile{"src/tasks/tesselate_terrain.glsl"}},

            .tesselation_control_shader_info =
                daxa::ShaderCompileInfo2{.source =
                                             daxa::ShaderFile{"src/tasks/tesselate_terrain.glsl"}},

            .tesselation_evaluation_shader_info =
                daxa::ShaderCompileInfo2{.source =
                                             daxa::ShaderFile{"src/tasks/tesselate_terrain.glsl"}},
            .fragment_shader_info =
                daxa::ShaderCompileInfo2{.source =
                                             daxa::ShaderFile{"src/tasks/tesselate_terrain.glsl"}},
            .color_attachments = {{.format = format}},
            .raster =
                {
                    .primitive_topology = daxa::PrimitiveTopology::PATCH_LIST,
                    // .polygon_mode = daxa::PolygonMode::LINE,
                    // .line_width = 0.05f,
                },
            .tesselation = {.control_points = 4},
            .push_constant_size = sizeof(TesselateTerrainPush),
        };
    };

    struct TesselateTerrainTask : TesselateTerrainH::Task
    {
        AttachmentViews views = {};
        daxa::RasterPipeline * pipeline = {};
        daxa::u32 vertex_count = 0;

        void callback(daxa::TaskInterface ti)
        {

            glm::mat4 view = glm::lookAt(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.15f, 0.0f),
                                         glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 proj =
                glm::perspective(glm::radians(70.0f),
                                 static_cast<float>(800) / static_cast<float>(600), 0.01f, 10.0f);
            proj[1][1] *= -1.0f;
            glm::mat4 proj_view = proj * view;

            auto image_info = ti.info(AT.dst_img).value();

            daxa::RenderCommandRecorder render_recorder =
                std::move(ti.recorder)
                    .begin_renderpass({
                        .color_attachments =
                            std::array{
                                daxa::RenderAttachmentInfo{
                                    .image_view = ti.view(AT.dst_img),
                                    .load_op = daxa::AttachmentLoadOp::CLEAR,
                                    .clear_value = std::array<daxa::f32, 4>{0.1f, 0.1f, 0.1f, 1.0f},
                                },
                            },
                        .render_area = {.width = image_info.size.x, .height = image_info.size.y},
                    });
            render_recorder.set_pipeline(*pipeline);

            render_recorder.push_constant(TesselateTerrainPush{
                .proj_view = std::bit_cast<daxa_f32mat4x4>(proj_view),
                .attachments = ti.attachment_shader_blob,
            });
            render_recorder.draw({.vertex_count = vertex_count});

            ti.recorder = std::move(render_recorder).end_renderpass();
        }
    };
} // namespace sylva
