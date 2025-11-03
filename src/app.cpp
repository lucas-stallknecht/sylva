#include "app.h"

#include <daxa/daxa.hpp>
#include <daxa/utils/task_graph.hpp>
#include <daxa/utils/task_graph_types.hpp>

#include "terrain/generation/terrain_generation.h"

namespace sylva
{
    App::App() : window_{"Sylva", 1600, 900}, ctx_{window_}
    {
        compile_pipelines();
        create_terrain_generation_task_graph();
        renderer_ = std::make_unique<Renderer>(ctx_, terrain_resources_);
    };

    App::~App()
    {
        ctx_.device.wait_idle();
        ctx_.device.destroy_image(terrain_resources_.height_map.get_state().images[0]);
        ctx_.device.destroy_image(terrain_resources_.albedo_map.get_state().images[0]);
        ctx_.device.collect_garbage();
    }

    void App::compile_pipelines()
    {
        terrain_gen_pipeline_ =
            ctx_.pipeline_manager
                .add_compute_pipeline2(sylva::generate_terrain_pipeline_compile_info())
                .value();
    }

    void App::create_terrain_generation_task_graph()
    {
        daxa::ImageId terrain_height_map_id = ctx_.device.create_image({
            .format = daxa::Format::R32_SFLOAT,
            .size = {.x = 4096, .y = 4096, .z = 1},
            .usage =
                daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::SHADER_SAMPLED,
        });
        daxa::ImageId terrain_albedo_id = ctx_.device.create_image({
            .format = daxa::Format::R8G8B8A8_UNORM,
            .size = {.x = 4096, .y = 4096, .z = 1},
            .usage =
                daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::SHADER_SAMPLED,
        });
        terrain_resources_.height_map = daxa::TaskImage({
            .initial_images = {.images = std::span{&terrain_height_map_id, 1}},
            .name = "task_terrain_height",
        });
        terrain_resources_.albedo_map = daxa::TaskImage({
            .initial_images = {.images = std::span{&terrain_albedo_id, 1}},
            .name = "task_terrain_albedo",
        });

        terrain_gen_tg_ = daxa::TaskGraph({
            .device = ctx_.device,
            .name = "generation_task_graph",
        });
        terrain_gen_tg_.use_persistent_image(terrain_resources_.height_map);
        terrain_gen_tg_.use_persistent_image(terrain_resources_.albedo_map);
        terrain_gen_tg_.add_task(
            daxa::HeadTask<GenerateTerrainH::Info>()
                .head_views({
                    .terrain_height_map = terrain_resources_.height_map.view(),
                    .terrain_albedo_map = terrain_resources_.albedo_map.view(),
                })
                .executes(sylva::generate_terrain_callback, terrain_gen_pipeline_.get()));
        terrain_gen_tg_.submit({});
        terrain_gen_tg_.complete({});
    }

    bool App::update()
    {
        if (window_.should_close())
        {
            return true;
        }
        window_.update();

        auto reload_result = ctx_.pipeline_manager.reload_all();

        if (auto * reload_err = daxa::get_if<daxa::PipelineReloadError>(&reload_result))
        {
            std::cout << reload_err->message << '\n';
        }
        else if (daxa::get_if<daxa::PipelineReloadSuccess>(&reload_result) != nullptr)
        {
            std::cout << "Shader reload success" << '\n';
        }

        if (window_.swapchain_out_of_date)
        {
            ctx_.swapchain.resize();
            window_.swapchain_out_of_date = false;
        }

        // TODO(lstallknecht): move this out the main loop once done iterating
        terrain_gen_tg_.execute({});
        renderer_->update();

        ctx_.device.collect_garbage();

        return false;
    }

} // namespace sylva
