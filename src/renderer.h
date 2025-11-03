#pragma once

#include <memory>
#include <unordered_map>
#include <daxa/utils/task_graph.hpp>

#include "gpu_context.h"
#include "terrain/rendering/terrain_rendering.h"
#include "terrain/terrain_common.h"

namespace sylva
{

    class Renderer
    {
      public:
        Renderer(GPUContext & gpu_context, TerrainResources & terrain_reousrces);
        Renderer(Renderer const &) = default;
        Renderer(Renderer &&) = delete;
        Renderer & operator=(Renderer const &) = delete;
        Renderer & operator=(Renderer &&) = delete;
        ~Renderer();

        void update();

      private:
        void compile_pipelines();
        void create_global_resources();
        void create_main_tg(TerrainResources & terrain_reousrces);

        GPUContext & ctx_;
        std::unordered_map<std::string, std::shared_ptr<daxa::RasterPipeline>> raster_pipelines_;
        daxa::TaskImage swapchain_image_;
        daxa::SamplerId linear_sampler_;
        daxa::TaskGraph main_tg_;
        RenderTerrainContext terrain_ctx_;
    };

} // namespace sylva
