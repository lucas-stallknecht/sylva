#pragma once

#include <daxa/daxa.hpp>
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/pipeline.hpp>

#include "window.h"

namespace sylva
{

    struct GPUContext
    {
        explicit GPUContext(Window const & window);
        GPUContext(GPUContext const &) = default;
        GPUContext(GPUContext &&) = delete;
        GPUContext & operator=(GPUContext const &) = default;
        GPUContext & operator=(GPUContext &&) = delete;
        ~GPUContext() = default;

        daxa::Instance instance;
        daxa::Device device;
        daxa::Swapchain swapchain;
        daxa::PipelineManager pipeline_manager;
    };

} // namespace sylva
