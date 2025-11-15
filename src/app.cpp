#include "app.h"

#include <daxa/daxa.hpp>
#include <daxa/utils/task_graph.hpp>
#include <daxa/utils/task_graph_types.hpp>
#include <imgui_impl_glfw.h>

#include "terrain/generation/terrain_generation.h"

namespace sylva
{
    App::App()
        : window_{"Sylva", defaults::window_width, defaults::window_height}, ctx_{window_},
          gui_{[&]()
               {
                   ImGui::CreateContext();
                   ImGui_ImplGlfw_InitForVulkan(window_.get_glfw_window(), true);
                   return daxa::ImGuiRenderer({
                       .device = ctx_.device,
                       .format = ctx_.swapchain.get_format(),
                   });
               }()}
    {
        compile_pipelines();
        create_terrain_generation_task_graph();
        renderer_ = std::make_unique<Renderer>(ctx_, camera_, terrain_resources_, gui_);
        generate_terrain();
    };

    App::~App()
    {
        ctx_.device.wait_idle();
        ctx_.device.collect_garbage();
        ImGui_ImplGlfw_Shutdown();
        ctx_.device.destroy_image(terrain_resources_.height_map.get_state().images[0]);
        ctx_.device.destroy_image(terrain_resources_.albedo_map.get_state().images[0]);
        ctx_.device.destroy_image(terrain_resources_.normal_map.get_state().images[0]);
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
            .size = {.x = 4096u, .y = 4096u, .z = 1u},
            .usage =
                daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::SHADER_SAMPLED,
        });
        daxa::ImageId terrain_albedo_id = ctx_.device.create_image({
            .format = daxa::Format::R8G8B8A8_UNORM,
            .size = {.x = 4096u, .y = 4096u, .z = 1u},
            .usage =
                daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::SHADER_SAMPLED,
        });
        daxa::ImageId terrain_normal_id = ctx_.device.create_image({
            .format = daxa::Format::R8G8B8A8_UNORM,
            .size = {.x = 4096u, .y = 4096u, .z = 1u},
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
        terrain_resources_.normal_map = daxa::TaskImage({
            .initial_images = {.images = std::span{&terrain_normal_id, 1}},
            .name = "task_terrain_normal",
        });

        terrain_gen_tg_ = daxa::TaskGraph({
            .device = ctx_.device,
            .name = "generation_task_graph",
        });
        terrain_gen_tg_.use_persistent_image(terrain_resources_.height_map);
        terrain_gen_tg_.use_persistent_image(terrain_resources_.albedo_map);
        terrain_gen_tg_.use_persistent_image(terrain_resources_.normal_map);
        terrain_gen_tg_.add_task(daxa::HeadTask<GenerateTerrainH::Info>()
                                     .head_views({
                                         .terrain_height_map = terrain_resources_.height_map.view(),
                                         .terrain_albedo_map = terrain_resources_.albedo_map.view(),
                                         .terrain_normal_map = terrain_resources_.normal_map.view(),
                                     })
                                     .executes(sylva::generate_terrain_callback,
                                               terrain_gen_pipeline_, &terrain_params_));
        terrain_gen_tg_.submit({});
        terrain_gen_tg_.complete({});
    }

    void App::generate_terrain() { terrain_gen_tg_.execute({}); }

    void App::ui_update()
    {
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Terrain Generation Parameters");

        bool has_terrain_params_changed = false;
        has_terrain_params_changed |=
            ImGui::SliderFloat("Amplitude", &terrain_params_.amplitude, 0.0f, 1.0f);
        has_terrain_params_changed |=
            ImGui::SliderFloat("Scale", &terrain_params_.scale, 0.0f, 2.0f);
        has_terrain_params_changed |= ImGui::SliderInt("Octaves", &terrain_params_.octaves, 1, 12);
        has_terrain_params_changed |=
            ImGui::SliderFloat("Persistence", &terrain_params_.persistence, 0.0f, 1.0f);
        has_terrain_params_changed |=
            ImGui::SliderFloat("Lacunarity", &terrain_params_.lacunarity, 0.0f, 4.0f);
        if (has_terrain_params_changed)
        {
            generate_terrain();
        }

        ImGui::End();

        ImGui::Render();
    }

    bool App::update()
    {
        if (window_.should_close())
        {
            return true;
        }
        auto & io = ImGui::GetIO();

        window_.update();
        camera_.process_input(window_, io.DeltaTime);
        ui_update();

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

        renderer_->update();

        ctx_.device.collect_garbage();

        return false;
    }

} // namespace sylva
