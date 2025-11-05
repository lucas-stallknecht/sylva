#include "renderer.h"

#include "terrain/rendering/terrain_rendering.h"

namespace sylva
{
    Renderer::Renderer(GPUContext & gpu_context, TerrainResources & terrain_reousrces,
                       daxa::ImGuiRenderer & gui)
        : ctx_{gpu_context}, terrain_ctx_(ctx_.device)
    {
        compile_pipelines();
        create_global_resources();
        create_main_tg(terrain_reousrces, gui);
    }

    Renderer::~Renderer()
    {
        ctx_.device.destroy_sampler(linear_sampler_);
        ctx_.device.destroy_buffer(terrain_ctx_.vertex_buffer.get_state().buffers[0]);
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
        swapchain_image_ = daxa::TaskImage({
            .swapchain_image = true,
            .name = "task_sc_image",
        });
    }

    void Renderer::create_main_tg(TerrainResources & terrain_reousrces, daxa::ImGuiRenderer & gui)
    {
        main_tg_ = daxa::TaskGraph({
            .device = ctx_.device,
            .swapchain = ctx_.swapchain,
            .name = "loop_task_graph",
        });

        main_tg_.use_persistent_image(swapchain_image_);

        main_tg_.use_persistent_image(terrain_reousrces.height_map);
        main_tg_.use_persistent_image(terrain_reousrces.albedo_map);

        main_tg_.use_persistent_buffer(terrain_ctx_.vertex_buffer);
        main_tg_.add_task(daxa::HeadTask<RenderTerrainH::Info>()
                              .head_views({
                                  .vertices = terrain_ctx_.vertex_buffer.view(),
                                  .terrain_height_map = terrain_reousrces.height_map.view(),
                                  .terrain_albedo_map = terrain_reousrces.albedo_map.view(),
                                  .dst_img = swapchain_image_.view(),
                              })
                              .executes(sylva::render_terrain_callback,
                                        raster_pipelines_.at("terrain_rendering").get(),
                                        &terrain_ctx_, linear_sampler_));

        auto imgui_task = daxa::InlineTask::Raster("Dear ImGui")
                              .color_attachment.reads_writes(swapchain_image_)
                              .executes(
                                  [&](daxa::TaskInterface ti)
                                  {
                                      auto render_target_id = ti.get(swapchain_image_).ids[0];
                                      auto render_size =
                                          ctx_.device.image_info(render_target_id).value().size;
                                      gui.record_commands(ImGui::GetDrawData(), ti.recorder,
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
