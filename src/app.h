#pragma once

#include <memory>
#include <daxa/daxa.hpp>
#include <daxa/utils/task_graph.hpp>
#include <daxa/utils/imgui.hpp>

#include "window.h"
#include "gpu_context.h"
#include "camera.h"
#include "renderer.h"
#include "terrain/terrain_common.h"
#include "terrain/generation/terrain_generation.inl"
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
    };
} // namespace sylva
