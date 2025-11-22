#include "renderer.h"

#include "rendering/raster_pipelines.h"
#include "utils/buffer_utils.h"

namespace sylva
{
    Renderer::Renderer(GPUContext & gpu_context, Camera const & camera,
                       TerrainResources const & terrain_resources, daxa::ImGuiRenderer & gui)
        : ctx_{gpu_context}
    {
        compile_pipelines();
        create_geometries();
        create_global_resources();
        create_main_tg(camera, terrain_resources, gui);
    }

    Renderer::~Renderer()
    {
        ctx_.device.destroy_image(depth_image_.get_state().images[0]);
        ctx_.device.destroy_sampler(linear_sampler_);

        for (auto & pair : geometries_)
        {
            ctx_.device.destroy_buffer(pair.second.vertex_buffer.get_state().buffers[0]);
        }
        geometries_.clear();

        ctx_.device.destroy_buffer(cam_buffer_.get_state().buffers[0]);
    }

    void Renderer::compile_pipelines()
    {
        raster_pipelines_.insert(
            {"terrain_rendering", ctx_.pipeline_manager
                                      .add_raster_pipeline2(tesselate_terrain_pipeline_compile_info(
                                          ctx_.swapchain.get_format()))
                                      .value()});
    }

    void Renderer::create_geometries()
    {
        auto terrain_vertices = generate_terrain_vertices(defaults::terrain_info);
        std::size_t terrain_vertex_count = terrain_vertices.size();

        geometries_.emplace(
            std::make_pair(std::string("terrain"),
                           Geometry{
                               .vertex_buffer = create_and_upload_buffer(
                                   ctx_.device, terrain_vertices.data(),
                                   daxa::BufferInfo{.size = terrain_vertex_count * sizeof(Vertex),
                                                    .name = "terrain_vertex_buffer"}),
                               .vertex_count = terrain_vertices.size(),
                           }));
    }

    void Renderer::create_global_resources()
    {
        linear_sampler_ = ctx_.device.create_sampler({
            .magnification_filter = daxa::Filter::LINEAR,
            .minification_filter = daxa::Filter::LINEAR,
        });
        auto cam_buffer_id =
            ctx_.device.create_buffer({.size = sizeof(CamInfo), .name = "camera_buffer"});
        cam_buffer_ = daxa::TaskBuffer({
            .initial_buffers = {.buffers = std::span{&cam_buffer_id, 1}},
            .name = "task_cam_buffer",
        });
        swapchain_image_ = daxa::TaskImage({
            .swapchain_image = true,
            .name = "task_sc_image",
        });
        daxa::Extent2D sc_extent = ctx_.swapchain.get_surface_extent();
        auto depth_image_id = ctx_.device.create_image({
            .format = daxa::Format::D32_SFLOAT,
            .size = {.x = sc_extent.x, .y = sc_extent.y, .z = 1u},
            .usage = daxa::ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT,
            .name = "depth_image",
        });
        depth_image_ = daxa::TaskImage({
            .initial_images = {.images = std::span{&depth_image_id, 1}},
            .name = "task_depth_image",
        });
    }

    void Renderer::create_main_tg(Camera const & camera, TerrainResources const & terrain_resources,
                                  daxa::ImGuiRenderer & gui)
    {
        // Capture pointers to the referenced parameters so lambdas stored in the task graph
        // do not hold references to stack variables (avoid potential dangling references).
        Camera const * camera_ptr = &camera;
        TerrainResources const * terrain_ptr = &terrain_resources;
        daxa::ImGuiRenderer * gui_ptr = &gui;

        main_tg_ = daxa::TaskGraph({
            .device = ctx_.device,
            .swapchain = ctx_.swapchain,
            .name = "loop_task_graph",
        });

        main_tg_.use_persistent_image(swapchain_image_);
        main_tg_.use_persistent_image(depth_image_);

        main_tg_.use_persistent_buffer(cam_buffer_);

        main_tg_.use_persistent_image(terrain_ptr->height_map);
        main_tg_.use_persistent_image(terrain_ptr->albedo_map);
        main_tg_.use_persistent_image(terrain_ptr->normal_map);

        main_tg_.add_task(
            daxa::InlineTask::Transfer("UploadCameraBuffer")
                .writes(cam_buffer_)
                .executes(
                    [camera_ptr, this](daxa::TaskInterface ti)
                    {
                        daxa::usize size = sizeof(CamInfo);
                        auto surface_extent = ctx_.swapchain.get_surface_extent();
                        auto proj_view =
                            camera_ptr->get_proj_view(static_cast<float>(surface_extent.x) /
                                                      static_cast<float>(surface_extent.y));

                        CamInfo cam_info{};
                        cam_info.position = std::bit_cast<daxa_f32vec3>(camera_ptr->get_position());
                        cam_info.proj_view = std::bit_cast<daxa_f32mat4x4>(proj_view);

                        upload_buffer(ti, cam_buffer_.get_state().buffers[0], &cam_info, size);
                    }));

        main_tg_.use_persistent_buffer(geometries_.at("terrain").vertex_buffer);
        main_tg_.add_task(
            daxa::HeadTask<DrawOpaqueH::Info>()
                .head_views({
                    .camera = cam_buffer_.view(),
                    .vertices = geometries_.at("terrain").vertex_buffer.view(),
                    .terrain_height_map = terrain_ptr->height_map.view(),
                    .terrain_albedo_map = terrain_ptr->albedo_map.view(),
                    .terrain_normal_map = terrain_ptr->normal_map.view(),
                    .dst_img = swapchain_image_.view(),
                    .depth_img = depth_image_.view(),
                })
                .executes(
                    [&](daxa::TaskInterface ti)
                    {
                        auto terrain_geo = geometries_.at("terrain");
                        auto const & AI = DrawOpaqueH::Info::AT;

                        auto image_info = ti.info(AI.dst_img).value();

                        daxa::RenderCommandRecorder render_recorder =
                            std::move(ti.recorder)
                                .begin_renderpass({
                                    .color_attachments =
                                        std::array{
                                            daxa::RenderAttachmentInfo{
                                                .image_view = ti.view(AI.dst_img),
                                                .load_op = daxa::AttachmentLoadOp::CLEAR,
                                                .clear_value = std::array<daxa::f32, 4>{0.1f, 0.1f,
                                                                                        0.1f, 1.0f},
                                            },
                                        },
                                    .depth_attachment =
                                        daxa::RenderAttachmentInfo{
                                            .image_view = ti.view(AI.depth_img),
                                            .load_op = daxa::AttachmentLoadOp::CLEAR,
                                            .clear_value = daxa::DepthValue{.depth = 1.0f},
                                        },
                                    .render_area = {.width = image_info.size.x,
                                                    .height = image_info.size.y},
                                });
                        render_recorder.set_pipeline(*raster_pipelines_.at("terrain_rendering"));

                        render_recorder.push_constant(DrawTerrainPush{
                            .linear_sampler = linear_sampler_,
                            .attachments = ti.attachment_shader_blob,
                        });
                        render_recorder.draw(
                            {.vertex_count = static_cast<daxa::u32>(terrain_geo.vertex_count)});

                        ti.recorder = std::move(render_recorder).end_renderpass();
                    }));

        auto imgui_task = daxa::InlineTask::Raster("DearImGui")
                              .color_attachment.reads_writes(swapchain_image_)
                              .executes(
                                  [gui_ptr, this](daxa::TaskInterface ti)
                                  {
                                      auto render_target_id = ti.get(swapchain_image_).ids[0];
                                      auto render_size =
                                          ti.device.image_info(render_target_id).value().size;
                                      gui_ptr->record_commands(ImGui::GetDrawData(), ti.recorder,
                                                               ti.get(swapchain_image_).ids[0],
                                                               render_size.x, render_size.y);
                                  });
        main_tg_.add_task(imgui_task);

        main_tg_.submit({});
        main_tg_.present({});
        main_tg_.complete({});
    }

    void Renderer::update()
    {
        auto next_swapchain_image = ctx_.swapchain.acquire_next_image();
        if (next_swapchain_image.is_empty())
        {
            return;
        }
        swapchain_image_.set_images({.images = std::span{&next_swapchain_image, 1}});

        main_tg_.execute({});
    }

} // namespace sylva
