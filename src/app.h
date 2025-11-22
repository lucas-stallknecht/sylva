#pragma once

#include <memory>
#include <daxa/daxa.hpp>
#include <daxa/utils/task_graph.hpp>
#include <daxa/utils/imgui.hpp>

#include "window.h"
#include "gpu_context.h"
#include "camera.h"
#include "renderer.h"
#include "common.h"
#include "defaults.h"

namespace sylva
{

    class App
    {
      public:
        explicit App();
        App(App const &) = delete;
        App(App &&) = delete;
        App & operator=(App const &) = delete;
        App & operator=(App &&) = delete;
        ~App();

        bool update();

      private:
        void compile_pipelines();
        void create_terrain_generation_task_graph();
        void generate_terrain();
        void create_grass_generation_task_graph();
        void generate_grass();
        void ui_update();

        // TODO(lstallknecht): use unique_ptr to avoid capturing pointers when creating task_graphs
        Window window_;
        GPUContext ctx_;
        Camera camera_;
        std::unique_ptr<Renderer> renderer_;
        daxa::ImGuiRenderer gui_;

        std::shared_ptr<daxa::ComputePipeline> terrain_gen_pipeline_;
        daxa::TaskGraph terrain_gen_tg_;
        TerrainResources terrain_resources_;
        TerrainGenerationParams terrain_params_ = defaults::terrain_generation_defaults;

        std::shared_ptr<daxa::ComputePipeline> grass_gen_pipeline_;
        daxa::TaskGraph grass_gen_tg_;
        std::shared_ptr<std::vector<GrassChunk>> grass_chunks_;
    };
} // namespace sylva
