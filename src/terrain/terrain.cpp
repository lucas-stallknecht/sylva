#include "terrain.h"

#include <daxa/daxa.hpp>

#include "../utils/upload_buffer.h"

namespace sylva
{

    std::vector<TerrainVertex> generate_terrain_vertices(TerrainInfo const & terrain_info)
    {
        std::vector<TerrainVertex> vertices;
        vertices.reserve(terrain_info.patches * terrain_info.patches * 4u);

        float const origin_x = -terrain_info.width_x * 0.5f;
        float const origin_y = -terrain_info.width_y * 0.5f;
        float const inv_res = 1.0f / static_cast<float>(terrain_info.patches);
        float const w_x = terrain_info.width_x;
        float const w_y = terrain_info.width_y;

        for (std::size_t i = 0; i < terrain_info.patches; ++i)
        {
            float const fi = static_cast<float>(i);
            float const base_x = origin_x + w_x * fi * inv_res;
            float const next_x = base_x + w_x * inv_res;

            float const u0 = fi * inv_res;
            float const u1 = (fi + 1.0f) * inv_res;

            for (std::size_t j = 0; j < terrain_info.patches; ++j)
            {
                float const fj = static_cast<float>(j);
                float const base_y = origin_y + w_y * fj * inv_res;
                float const next_y = base_y + w_y * inv_res;

                float const v0 = fj * inv_res;
                float const v1 = (fj + 1.0f) * inv_res;

                vertices.push_back({.position = {base_x, 0.0f, base_y}, .uv = {u0, v0}});
                vertices.push_back({.position = {next_x, 0.0f, base_y}, .uv = {u1, v0}});
                vertices.push_back({.position = {base_x, 0.0f, next_y}, .uv = {u0, v1}});
                vertices.push_back({.position = {next_x, 0.0f, next_y}, .uv = {u1, v1}});
            }
        }

        return vertices;
    }

    void TerrainContext::update_from_terrain_info(daxa::Device device,
                                                  TerrainInfo const & terrain_info)
    {
        auto vertices = generate_terrain_vertices(terrain_info);
        daxa::usize size = vertices.size() * sizeof(TerrainVertex);
        vertex_count = static_cast<daxa::u32>(vertices.size());

        daxa::BufferInfo vertex_buffer_info = {
            .size = size,
            .name = "terrain_vertex_buffer",
        };

        if (!vertex_buffer.is_valid())
        {
            vertex_buffer = upload_buffer(device, vertices.data(), vertex_buffer_info);
            return;
        }
        device.destroy_buffer(vertex_buffer.get_state().buffers[0]);

        daxa::BufferId buffer_id = device.create_buffer(vertex_buffer_info);
        upload_buffer(device, buffer_id, vertices.data(), size, "terrain_vertex_buffer");
        vertex_buffer.set_buffers({.buffers = std::span{&buffer_id, 1}});
    }

    void TesselateTerrainTask::callback(daxa::TaskInterface ti)
    {

        glm::mat4 view = glm::lookAt(glm::vec3(0.3f, 0.1f, 0.3f), glm::vec3(0.0f, 0.0f, 0.0f),
                                     glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 proj = glm::perspective(
            glm::radians(70.0f), static_cast<float>(1600) / static_cast<float>(900), 0.01f, 10.0f);
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
            .linear_sampler = sampler,
            .attachments = ti.attachment_shader_blob,
        });
        render_recorder.draw({.vertex_count = terrain_context->vertex_count});

        ti.recorder = std::move(render_recorder).end_renderpass();
    }

} // namespace sylva
