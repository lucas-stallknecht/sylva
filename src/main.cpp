#define STB_IMAGE_IMPLEMENTATION

#include <daxa/daxa.hpp>
#include <daxa/utils/task_graph.hpp>
#include <daxa/utils/task_graph_types.hpp>

#include "gpu_context.h"
#include "window.h"
#include "terrain/rendering/terrain_rendering.h"
#include "terrain/generation/terrain_generation.h"
#include "terrain/generation/terrain_generation.inl"

/* === ROADMAP ===
 * [/] Generate terrain with noise -> 3 maps (albedo, height, normal)
 * [/] Render tesselated terrain + apply dynamic tesselation levels
 * [ ] Add ImGui
 * [ ] Generate grass chunks => grass patches => grass blade parameters
 * [ ] Render grass blades
 * [ ] Simulate wind
 * [ ] Shade grass blades (SSS)
 * [ ] Apply optimization techniques -> Culling, LODs
 * [ ] Render moss and other foilage
 */

int main()
{
    sylva::Window window("Sylva", 1600, 900);
    sylva::GPUContext context(window);

    // Generation
    auto generation_pipeline =
        context.pipeline_manager
            .add_compute_pipeline2(sylva::generate_terrain_pipeline_compile_info())
            .value();

    daxa::ImageId terrain_height_map_id = context.device.create_image({
        .format = daxa::Format::R32_SFLOAT,
        .size = {.x = 4096, .y = 4096, .z = 1},
        .usage =
            daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::SHADER_SAMPLED,
        .name = "terrain_height",
    });
    daxa::ImageId terrain_albedo_id = context.device.create_image({
        .format = daxa::Format::R8G8B8A8_UNORM,
        .size = {.x = 4096, .y = 4096, .z = 1},
        .usage =
            daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::SHADER_SAMPLED,
        .name = "terrain_albedo",
    });
    auto terrain_height = daxa::TaskImage({
        .initial_images = {.images = std::span{&terrain_height_map_id, 1}},
        .name = "task_terrain_height",
    });
    auto terrain_albedo = daxa::TaskImage({
        .initial_images = {.images = std::span{&terrain_albedo_id, 1}},
        .name = "task_terrain_albedo",
    });

    auto gen_tg = daxa::TaskGraph({
        .device = context.device,
        .name = "generation_task_graph",
    });

    gen_tg.use_persistent_image(terrain_height);
    gen_tg.use_persistent_image(terrain_albedo);
    gen_tg.add_task(daxa::HeadTask<GenerateTerrainH::Info>()
                        .head_views({
                            .terrain_height_map = terrain_height.view(),
                            .terrain_albedo_map = terrain_albedo.view(),
                        })
                        .executes(sylva::generate_terrain_callback, generation_pipeline.get()));

    gen_tg.submit({});
    gen_tg.complete({});

    // Pipeline
    auto terrain_pipeline =
        context.pipeline_manager
            .add_raster_pipeline2(
                sylva::tesselate_terrain_pipeline_compile_info(context.swapchain.get_format()))
            .value();

    // Sampler
    auto linear_sampler = context.device.create_sampler({
        .magnification_filter = daxa::Filter::LINEAR,
        .minification_filter = daxa::Filter::LINEAR,
    });

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

    sylva::RenderTerrainContext terrain_context(context.device);
    tg.use_persistent_buffer(terrain_context.vertex_buffer);
    tg.add_task(daxa::HeadTask<RenderTerrainH::Info>()
                    .head_views({
                        .vertices = terrain_context.vertex_buffer.view(),
                        .terrain_height_map = terrain_height.view(),
                        .terrain_albedo_map = terrain_albedo.view(),
                        .dst_img = task_swapchain_image.view(),
                    })
                    .executes(sylva::render_terrain_callback, terrain_pipeline.get(),
                              &terrain_context, linear_sampler));

    tg.submit({});
    tg.present({});
    tg.complete({});

    // Main loop
    while (!window.should_close())
    {
        window.update();

        auto reload_result = context.pipeline_manager.reload_all();

        if (auto * reload_err = daxa::get_if<daxa::PipelineReloadError>(&reload_result))
        {
            std::cout << reload_err->message << '\n';
        }
        else if (daxa::get_if<daxa::PipelineReloadSuccess>(&reload_result) != nullptr)
        {
            std::cout << "Shader reload success" << '\n';
        }

        if (window.swapchain_out_of_date)
        {
            context.swapchain.resize();
            window.swapchain_out_of_date = false;
        }

        auto swapchain_image = context.swapchain.acquire_next_image();
        if (swapchain_image.is_empty())
        {
            continue;
        }
        task_swapchain_image.set_images({.images = std::span{&swapchain_image, 1}});

        // TODO(lstallknecht): move this out the main loop once done iterating
        gen_tg.execute({});
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
