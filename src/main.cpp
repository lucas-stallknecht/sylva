#define STB_IMAGE_IMPLEMENTATION

#include <iostream>

#include <daxa/daxa.hpp>
#include <daxa/utils/task_graph.hpp>
#include <daxa/utils/task_graph_types.hpp>

#include "gpu_context.h"
#include "window.h"
#include "terrain/terrain.h"
#include "utils/upload_image.h"

int main()
{
    sylva::Window window("Sylva", 1600, 900);
    sylva::GPUContext context(window);

    // Pipeline
    auto pipeline_res = context.pipeline_manager.add_raster_pipeline2(
        sylva::tesselate_terrain_pipeline_compile_info(context.swapchain.get_format()));
    if (pipeline_res.is_err())
    {
        std::cerr << "Failed to create raster pipeline: " << pipeline_res.message() << std::endl;
        return 1;
    }
    auto terrain_pipeline = pipeline_res.value();

    // Sampler
    auto linear_sampler = context.device.create_sampler({
        .magnification_filter = daxa::Filter::LINEAR,
        .minification_filter = daxa::Filter::LINEAR,
    });

    // Resources
    auto hm_opt = sylva::upload_image(context.device, "resources/textures/terrain_height.hdr",
                                      daxa::ImageInfo{
                                          .format = daxa::Format::R32_SFLOAT,
                                          .usage = daxa::ImageUsageFlagBits::TRANSFER_DST |
                                                   daxa::ImageUsageFlagBits::SHADER_STORAGE |
                                                   daxa::ImageUsageFlagBits::SHADER_SAMPLED,
                                          .name = "height_map_image",
                                      });
    if (!hm_opt.has_value())
    {
        std::cerr << "Failed to upload height map " << std::endl;
        return 1;
    }
    daxa::TaskImage terrain_height = hm_opt.value();

    auto albedo_opt = sylva::upload_image(context.device, "resources/textures/terrain_albedo.png",
                                          daxa::ImageInfo{
                                              .format = daxa::Format::R8G8B8A8_UNORM,
                                              .usage = daxa::ImageUsageFlagBits::TRANSFER_DST |
                                                       daxa::ImageUsageFlagBits::SHADER_STORAGE |
                                                       daxa::ImageUsageFlagBits::SHADER_SAMPLED,
                                              .name = "terrain_color_image",
                                          });
    if (!albedo_opt.has_value())
    {
        std::cerr << "Failed to upload height map " << std::endl;
        return 1;
    }
    daxa::TaskImage terrain_albedo = albedo_opt.value();

    // Task graph
    auto tg = daxa::TaskGraph({
        .device = context.device,
        .swapchain = context.swapchain,
        .name = "loop_task_graph",
    });

    auto task_swapchain_image = daxa::TaskImage({
        .swapchain_image = true,
        .name = "task_sc_image",
    });
    tg.use_persistent_image(task_swapchain_image);

    tg.use_persistent_image(terrain_height);
    tg.use_persistent_image(terrain_albedo);

    sylva::TerrainContext terrain_context(context.device);
    tg.use_persistent_buffer(terrain_context.vertex_buffer);
    tg.add_task(sylva::TesselateTerrainTask{
        .views =
            {
                .vertices = terrain_context.vertex_buffer.view(),
                .terrain_height_map = terrain_height.view(),
                .terrain_albedo_map = terrain_albedo.view(),
                .dst_img = task_swapchain_image.view(),
            },
        .pipeline = terrain_pipeline.get(),
        .terrain_context = &terrain_context,
        .sampler = linear_sampler,
    });

    tg.submit({});
    tg.present({});
    tg.complete({});

    // Main loop
    while (!window.should_close())
    {
        window.update();

        if (window.swapchain_out_of_date)
        {
            context.swapchain.resize();
            window.swapchain_out_of_date = false;
        }

        auto swapchain_image = context.swapchain.acquire_next_image();
        if (swapchain_image.is_empty())
            continue;
        task_swapchain_image.set_images({.images = std::span{&swapchain_image, 1}});

        tg.execute({});
        context.device.collect_garbage();
    }

    context.device.wait_idle();

    // Cleanup
    context.device.destroy_buffer(terrain_context.vertex_buffer.get_state().buffers[0]);
    context.device.destroy_image(terrain_height.get_state().images[0]);
    context.device.destroy_image(terrain_albedo.get_state().images[0]);
    context.device.destroy_sampler(linear_sampler);
    context.device.collect_garbage();

    return 0;
}
