#pragma once

#include <memory>
#include <unordered_map>
#include <daxa/utils/task_graph.hpp>
#include <daxa/utils/imgui.hpp>

#include "camera.h"
#include "gpu_context.h"
#include "terrain/terrain_common.h"

namespace sylva
{

    class Renderer
    {
      public:
        Renderer(GPUContext & gpu_context, Camera const & camera,
                 TerrainResources const & terrain_resources, daxa::ImGuiRenderer & gui);
        Renderer(Renderer const &) = delete;
        Renderer(Renderer &&) = delete;
        Renderer & operator=(Renderer const &) = delete;
        Renderer & operator=(Renderer &&) = delete;
        ~Renderer();

        void update();

      private:
        void compile_pipelines();
        void create_geometries();
        void create_global_resources();
        void create_main_tg(Camera const & camera, TerrainResources const & terrain_resources,
                            daxa::ImGuiRenderer & gui);
        void render_terrain(TerrainResources const * terrain_ptr);

        GPUContext & ctx_;
        std::unordered_map<std::string, std::shared_ptr<daxa::RasterPipeline>> raster_pipelines_;
        daxa::TaskImage swapchain_image_;
        daxa::TaskImage depth_image_;
        daxa::TaskBuffer cam_buffer_;
        daxa::SamplerId linear_sampler_;
        daxa::TaskGraph main_tg_;
        // TODO(lstallknecht): this should somewhat be out of the renderer class
        std::unordered_map<std::string, Geometry> geometries_;
    };

} // namespace sylva
