#include "terrain_rendering.h"

#include <daxa/daxa.hpp>

#include "../../utils/buffer_utils.h"

namespace sylva
{

    std::vector<TerrainVertex> generate_terrain_vertices(TerrainInfo const & terrain_info)
    {
        std::vector<TerrainVertex> vertices;
        vertices.reserve(terrain_info.patch_grid_size * terrain_info.patch_grid_size * 4u);

        float const origin_x = -terrain_info.patch_width * 0.5f;
        float const origin_z = -terrain_info.patch_width * 0.5f;
        float const inv_res = 1.0f / static_cast<float>(terrain_info.patch_grid_size);
        float const w_x = terrain_info.patch_width;
        float const w_z = terrain_info.patch_width;

        for (std::size_t i = 0; i < terrain_info.patch_grid_size; ++i)
        {
            auto const fi = static_cast<float>(i);
            float const base_x = origin_x + (w_x * fi * inv_res);
            float const next_x = base_x + (w_x * inv_res);

            float const u0 = fi * inv_res;
            float const u1 = (fi + 1.0f) * inv_res;

            for (std::size_t j = 0; j < terrain_info.patch_grid_size; ++j)
            {
                auto const fj = static_cast<float>(j);
                float const base_z = origin_z + (w_z * fj * inv_res);
                float const next_z = base_z + (w_z * inv_res);

                float const v0 = fj * inv_res;
                float const v1 = (fj + 1.0f) * inv_res;

                vertices.push_back({.position = {base_x, 0.0f, base_z}, .uv = {u0, v0}});
                vertices.push_back({.position = {next_x, 0.0f, base_z}, .uv = {u1, v0}});
                vertices.push_back({.position = {base_x, 0.0f, next_z}, .uv = {u0, v1}});
                vertices.push_back({.position = {next_x, 0.0f, next_z}, .uv = {u1, v1}});
            }
        }

        return vertices;
    }

    void TerrainGeometry::update_from_terrain_info(daxa::Device & device,
                                                   TerrainInfo const & terrain_info)
    {
        auto vertices = generate_terrain_vertices(terrain_info);

        // Compute buffer size in bytes explicitly and use daxa::usize for API calls.
        auto buffer_size_bytes = static_cast<daxa::usize>(vertices.size() * sizeof(TerrainVertex));
        vertex_count = vertices.size();

        daxa::BufferInfo vertex_buffer_info = {
            .size = buffer_size_bytes,
            .name = "terrain_vertex_buffer",
        };

        if (!vertex_buffer.is_valid())
        {
            vertex_buffer = create_and_upload_buffer(device, vertices.data(), vertex_buffer_info);
            return;
        }
        device.destroy_buffer(vertex_buffer.get_state().buffers[0]);

        daxa::BufferId buffer_id = device.create_buffer(vertex_buffer_info);
        create_and_upload_buffer(device, buffer_id, vertices.data(), buffer_size_bytes,
                                 "terrain_vertex_buffer");
        vertex_buffer.set_buffers({.buffers = std::span{&buffer_id, 1}});
    }

    void render_terrain_callback(daxa::TaskInterface ti,
                                 std::shared_ptr<daxa::RasterPipeline> const & pipeline,
                                 std::size_t vertex_count, daxa::SamplerId sampler)
    {
        auto const & AI = RenderTerrainH::Info::AT;

        auto image_info = ti.info(AI.dst_img).value();

        daxa::RenderCommandRecorder render_recorder =
            std::move(ti.recorder)
                .begin_renderpass({
                    .color_attachments =
                        std::array{
                            daxa::RenderAttachmentInfo{
                                .image_view = ti.view(AI.dst_img),
                                .load_op = daxa::AttachmentLoadOp::CLEAR,
                                .clear_value = std::array<daxa::f32, 4>{0.1f, 0.1f, 0.1f, 1.0f},
                            },
                        },
                    .depth_attachment =
                        daxa::RenderAttachmentInfo{
                            .image_view = ti.view(AI.depth),
                            .load_op = daxa::AttachmentLoadOp::CLEAR,
                            .clear_value = daxa::DepthValue{.depth = 1.0f},
                        },
                    .render_area = {.width = image_info.size.x, .height = image_info.size.y},
                });
        render_recorder.set_pipeline(*pipeline);

        render_recorder.push_constant(RenderTerrainPush{
            .linear_sampler = sampler,
            .attachments = ti.attachment_shader_blob,
        });
        render_recorder.draw({.vertex_count = static_cast<daxa::u32>(vertex_count)});

        ti.recorder = std::move(render_recorder).end_renderpass();
    }

} // namespace sylva
