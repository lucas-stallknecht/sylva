#include "gpu_context.h"

#include <daxa/utils/pipeline_manager.hpp>

namespace sylva
{

    GPUContext::GPUContext(Window const & window)
        : instance{daxa::create_instance({})},
          device{instance.create_device_2(instance.choose_device({}, {}))},
          swapchain{device.create_swapchain({
              .native_window = window.get_native_window_handle(),
              .native_window_platform = window.get_native_platform(),
              .present_mode = daxa::PresentMode::FIFO,
              .image_usage =
                  daxa::ImageUsageFlagBits::TRANSFER_DST | daxa::ImageUsageFlagBits::SHADER_STORAGE,
          })},
          pipeline_manager{daxa::PipelineManager({
              .device = device,
              .root_paths =
                  {
                      DAXA_SHADER_INCLUDE_DIR,
                      "src",
                  },
              .default_language = daxa::ShaderLanguage::GLSL,
          })}
    {
    }

} // namespace sylva
