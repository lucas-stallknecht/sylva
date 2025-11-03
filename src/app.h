#pragma once

#include <memory>
#include <daxa/daxa.hpp>
#include <daxa/utils/task_graph.hpp>

#include "window.h"
#include "gpu_context.h"
#include "renderer.h"
#include "terrain/terrain_common.h"

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

        Window window_;
        GPUContext ctx_;
        std::unique_ptr<Renderer> renderer_;

        std::shared_ptr<daxa::ComputePipeline> terrain_gen_pipeline_;
        daxa::TaskGraph terrain_gen_tg_;
        TerrainResources terrain_resources_;
    };
} // namespace sylva
