#include "app.h"

#include <string>
#include <daxa/daxa.hpp>
#include <daxa/utils/task_graph.hpp>
#include <daxa/utils/task_graph_types.hpp>
#include <imgui_impl_glfw.h>

#include "generation/terrain_generation.h"
#include "generation/grass_generation.h"

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
        create_grass_generation_task_graph();
        renderer_ = std::make_unique<Renderer>(ctx_, camera_, scene_, gui_);
        generate_terrain();
        generate_grass();
    };

    App::~App()
    {
        ctx_.device.wait_idle();
        ctx_.device.collect_garbage();
        ImGui_ImplGlfw_Shutdown();
        ctx_.device.destroy_image(scene_.terrain_height_map.get_state().images[0]);
        ctx_.device.destroy_image(scene_.terrain_albedo_map.get_state().images[0]);
        ctx_.device.destroy_image(scene_.terrain_normal_map.get_state().images[0]);
        for (auto & chunk : *scene_.grass_chunks)
        {
            ctx_.device.destroy_buffer(chunk.blade_buffer_id);
        }
    }

    void App::compile_pipelines()
    {
        terrain_gen_pipeline_ =
            ctx_.pipeline_manager.add_compute_pipeline2(generate_terrain_pipeline_compile_info())
                .value();
        grass_gen_pipeline_ =
            ctx_.pipeline_manager.add_compute_pipeline2(generate_grass_pipeline_compile_info())
                .value();
    }

    void App::create_terrain_generation_task_graph()
    {
        daxa::ImageId terrain_height_map_id = ctx_.device.create_image({
            .format = daxa::Format::R32_SFLOAT,
            .size =
                {
                    .x = defaults::terrain_map_resolution,
                    .y = defaults::terrain_map_resolution,
                    .z = 1u,
                },
            .usage =
                daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::SHADER_SAMPLED,
        });
        daxa::ImageId terrain_albedo_id = ctx_.device.create_image({
            .format = daxa::Format::R8G8B8A8_UNORM,
            .size =
                {
                    .x = defaults::terrain_map_resolution,
                    .y = defaults::terrain_map_resolution,
                    .z = 1u,
                },
            .usage =
                daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::SHADER_SAMPLED,
        });
        daxa::ImageId terrain_normal_id = ctx_.device.create_image({
            .format = daxa::Format::R8G8B8A8_UNORM,
            .size =
                {
                    .x = defaults::terrain_map_resolution,
                    .y = defaults::terrain_map_resolution,
                    .z = 1u,
                },
            .usage =
                daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::SHADER_SAMPLED,
        });
        scene_.terrain_height_map = daxa::TaskImage({
            .initial_images = {.images = std::span{&terrain_height_map_id, 1}},
            .name = "task_terrain_height",
        });
        scene_.terrain_albedo_map = daxa::TaskImage({
            .initial_images = {.images = std::span{&terrain_albedo_id, 1}},
            .name = "task_terrain_albedo",
        });
        scene_.terrain_normal_map = daxa::TaskImage({
            .initial_images = {.images = std::span{&terrain_normal_id, 1}},
            .name = "task_terrain_normal",
        });

        terrain_gen_tg_ = daxa::TaskGraph({
            .device = ctx_.device,
            .name = "terrain_generation_task_graph",
        });
        terrain_gen_tg_.use_persistent_image(scene_.terrain_height_map);
        terrain_gen_tg_.use_persistent_image(scene_.terrain_albedo_map);
        terrain_gen_tg_.use_persistent_image(scene_.terrain_normal_map);
        terrain_gen_tg_.add_task(
            daxa::HeadTask<GenerateTerrainH::Info>()
                .head_views({
                    .terrain_height_map = scene_.terrain_height_map.view(),
                    .terrain_albedo_map = scene_.terrain_albedo_map.view(),
                    .terrain_normal_map = scene_.terrain_normal_map.view(),
                })
                .executes(generate_terrain_callback, terrain_gen_pipeline_, &terrain_params_));
        terrain_gen_tg_.submit({});
        terrain_gen_tg_.complete({});
    }

    void App::generate_terrain() { terrain_gen_tg_.execute({}); }

    void App::create_grass_generation_task_graph()
    {
        // Params
        auto const params = defaults::grass_chunk_params;
        scene_.grass_chunks = create_grass_chunk_positions(params);

        float const blade_step = params.chunk_width / params.blade_density;
        auto const blades_per_side =
            static_cast<uint32_t>(std::floor(params.chunk_width * params.blade_density));
        std::uint32_t const blades_per_chunk = blades_per_side * blades_per_side;
        std::size_t const blade_buffer_size = blades_per_chunk * sizeof(GrassBlade);

        // TODO(lstallknecht): this is computed directly from defaults; replace with a proper
        // terrain rendering parameter once available
        float const terrain_total_width =
            defaults::terrain_info.patch_width * defaults::terrain_info.patch_grid_size;

        // Chunk buffers
        std::vector<daxa::BufferId> buffer_ids;
        buffer_ids.reserve(scene_.grass_chunks->size());
        for (auto & chunk : *scene_.grass_chunks)
        {
            chunk.blade_buffer_id = ctx_.device.create_buffer(
                {.size = blade_buffer_size, .name = "blade_buffer_" + std::to_string(chunk.seed)});
            chunk.blade_count = blades_per_chunk;

            buffer_ids.push_back(chunk.blade_buffer_id);
        }
        scene_.grass_blades_buffer = daxa::TaskBuffer({
            .initial_buffers = {.buffers = std::span{buffer_ids}},
            .name = "task_grass_blades_buffer",
        });

        grass_gen_tg_ = daxa::TaskGraph({
            .device = ctx_.device,
            .name = "grass_generation_task_graph",
        });

        grass_gen_tg_.use_persistent_image(scene_.terrain_height_map);
        grass_gen_tg_.use_persistent_buffer(scene_.grass_blades_buffer);
        grass_gen_tg_.add_task(daxa::HeadTask<GenerateGrassBladesH::Info>()
                                   .head_views({
                                       .terrain_height_map = scene_.terrain_height_map.view(),
                                       .blades = scene_.grass_blades_buffer.view(),
                                   })
                                   .executes(generate_grass_callback, grass_gen_pipeline_,
                                             GrassGenerationParams{
                                                 .grass_chunks = scene_.grass_chunks,
                                                 .blade_step = blade_step,
                                                 .blades_per_side = blades_per_side,
                                                 .terrain_total_width = terrain_total_width,
                                             }));
        grass_gen_tg_.submit({});
        grass_gen_tg_.complete({});
    }

    void App::generate_grass() { grass_gen_tg_.execute({}); }

    void App::ui_update()
    {
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Terrain Generation Parameters");

        bool has_terrain_params_changed = false;
        has_terrain_params_changed |=
            ImGui::SliderFloat("Amplitude", &terrain_params_.amplitude, 0.01f, 4.0f);
        has_terrain_params_changed |=
            ImGui::SliderFloat("Scale", &terrain_params_.scale, 0.01f, 2.0f);
        std::uint32_t octaves_min = 1u;
        std::uint32_t octaves_max = 14u;
        has_terrain_params_changed |= ImGui::SliderScalar(
            "Octaves", ImGuiDataType_U32, &terrain_params_.octaves, &octaves_min, &octaves_max);
        has_terrain_params_changed |=
            ImGui::SliderFloat("Persistence", &terrain_params_.persistence, 0.0f, 1.0f);
        has_terrain_params_changed |=
            ImGui::SliderFloat("Lacunarity", &terrain_params_.lacunarity, 0.0f, 4.0f);

        has_terrain_params_changed |=
            ImGui::SliderFloat("Slope Min", &terrain_params_.slope_min, 0.01f, 1.0f);
        has_terrain_params_changed |=
            ImGui::SliderFloat("Slope Max", &terrain_params_.slope_max, 0.01f, 1.0f);

        // Clamp min to 0..max
        if (terrain_params_.slope_min > terrain_params_.slope_max)
        {
            terrain_params_.slope_min = terrain_params_.slope_max;
            has_terrain_params_changed = true;
        }

        if (has_terrain_params_changed)
        {
            generate_terrain();
            generate_grass();
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
