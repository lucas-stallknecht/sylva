#pragma once

#include <memory>
#include <unordered_map>
#include <daxa/daxa.hpp>
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/pipeline.hpp>

#include "window.h"

namespace sylva
{

    struct GPUContext
    {
        explicit GPUContext(Window const & window);
        ~GPUContext() = default;

        daxa::Instance instance = {};
        daxa::Device device = {};
        daxa::Swapchain swapchain = {};
        daxa::PipelineManager pipeline_manager = {};

        std::unordered_map<std::string, std::shared_ptr<daxa::RasterPipeline>> raster_pipelines =
            {};
    };

} // namespace sylva
