#include "renderer.h"

#include "terrain/rendering/terrain_rendering.h"

namespace sylva
{
    Renderer::Renderer(GPUContext & gpu_context, Camera const & camera,
                       TerrainResources const & terrain_resources, daxa::ImGuiRenderer & gui)
        : ctx_{gpu_context}, terrain_ctx_(ctx_.device)
    {
        compile_pipelines();
        create_global_resources();
        create_main_tg(camera, terrain_resources, gui);
    }

    Renderer::~Renderer()
    {
        ctx_.device.destroy_sampler(linear_sampler_);
        ctx_.device.destroy_buffer(terrain_ctx_.vertex_buffer.get_state().buffers[0]);
        ctx_.device.destroy_buffer(cam_buffer_.get_state().buffers[0]);
    }

    void Renderer::compile_pipelines()
    {
        raster_pipelines_.insert(
            {"terrain_rendering",
             ctx_.pipeline_manager
                 .add_raster_pipeline2(
                     sylva::tesselate_terrain_pipeline_compile_info(ctx_.swapchain.get_format()))
                 .value()});
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

        main_tg_.use_persistent_buffer(cam_buffer_);

        main_tg_.use_persistent_image(terrain_ptr->height_map);
        main_tg_.use_persistent_image(terrain_ptr->albedo_map);
        main_tg_.use_persistent_buffer(terrain_ctx_.vertex_buffer);

        main_tg_.add_task(
            daxa::InlineTask::Transfer("Upload Camera Buffer")
                .writes(cam_buffer_)
                .executes(
                    [camera_ptr, this](daxa::TaskInterface ti)
                    {
                        daxa::usize size = sizeof(CamInfo);
                        daxa::BufferId staging_buffer_id = ti.device.create_buffer({
                            .size = size,
                            .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE,
                            .name = "cam_staging_buffer",
                        });
                        ti.recorder.destroy_buffer_deferred(staging_buffer_id);

                        auto surface_extent = ctx_.swapchain.get_surface_extent();
                        auto proj_view =
                            camera_ptr->get_proj_view(static_cast<float>(surface_extent.x) /
                                                      static_cast<float>(surface_extent.y));

                        CamInfo cam_info{};
                        cam_info.proj_view = std::bit_cast<daxa_f32mat4x4>(proj_view);

                        void * mapped =
                            ti.device.buffer_host_address_as<void *>(staging_buffer_id).value();
                        std::memcpy(mapped, &cam_info, sizeof(cam_info));

                        ti.recorder.copy_buffer_to_buffer({
                            .src_buffer = staging_buffer_id,
                            .dst_buffer = ti.get(cam_buffer_).ids[0],
                            .size = size,
                        });
                    }));

        main_tg_.add_task(daxa::HeadTask<RenderTerrainH::Info>()
                              .head_views({
                                  .vertices = terrain_ctx_.vertex_buffer.view(),
                                  .terrain_height_map = terrain_ptr->height_map.view(),
                                  .terrain_albedo_map = terrain_ptr->albedo_map.view(),
                                  .dst_img = swapchain_image_.view(),
                              })
                              .executes(sylva::render_terrain_callback,
                                        raster_pipelines_.at("terrain_rendering"), &terrain_ctx_,
                                        linear_sampler_, cam_buffer_));

        auto imgui_task = daxa::InlineTask::Raster("Dear ImGui")
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
