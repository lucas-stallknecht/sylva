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
        void ui_update();

        Window window_;
        GPUContext ctx_;
        Camera camera_;
        std::unique_ptr<Renderer> renderer_;
        daxa::ImGuiRenderer gui_;

        std::shared_ptr<daxa::ComputePipeline> terrain_gen_pipeline_;
        daxa::TaskGraph terrain_gen_tg_;
        TerrainResources terrain_resources_;
        // TODO(lstallknecht): move the default parameters in a config file
        TerrainGenerationParams terrain_params_ = {
            .amplitude = 0.5f,
            .scale = 0.7f,
            .octaves = 7,
            .persistence = 0.4f,
            .lacunarity = 2.0f,
        };
    };
} // namespace sylva
