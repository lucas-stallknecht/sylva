#include "buffer_utils.h"

namespace sylva
{
    void upload_buffer(daxa::TaskInterface & task_interface, daxa::BufferId buffer_id,
                       void const * data, daxa::usize size)
    {
        daxa::BufferId staging_buffer_id = task_interface.device.create_buffer({
            .size = size,
            .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE,
            .name = "staging_buffer",
        });
        task_interface.recorder.destroy_buffer_deferred(staging_buffer_id);

        void * mapped = task_interface.device.buffer_host_address(staging_buffer_id).value();
        std::memcpy(mapped, data, size);

        task_interface.recorder.copy_buffer_to_buffer({
            .src_buffer = staging_buffer_id,
            .dst_buffer = buffer_id,
            .size = size,
        });
    }

    // TODO(lstallknecht): Make upload TaskGraph reusable for multiple uploads
    // Currently, we create a new TaskGraph per upload which is inefficient
    // Instead, we should build it once and reuse it with parameterised data and buffers
    // Maybe with a uploader class located in the gpu_context ?
    UploadedBufferResult create_and_upload_buffer(daxa::Device & device, daxa::BufferId buffer_id,
                                                  void const * data, daxa::usize size,
                                                  std::string name)
    {
        daxa::TaskBuffer task_buffer({
            .initial_buffers = {.buffers = std::span{&buffer_id, 1}},
            .name = std::move(name),
        });

        // one-time upload graph
        daxa::TaskGraph upload_tg({
            .device = device,
            .name = name + "_task_graph",
        });

        upload_tg.use_persistent_buffer(task_buffer);
        upload_tg.add_task(daxa::InlineTask::Transfer("UploadBuffer")
                               .writes(daxa::TaskStage::TRANSFER, task_buffer)
                               .executes([=](daxa::TaskInterface ti)
                                         { upload_buffer(ti, buffer_id, data, size); }));

        upload_tg.submit({});
        upload_tg.complete({});
        upload_tg.execute({});

        return UploadedBufferResult{.buffer_id = buffer_id, .task_buffer = task_buffer};
    }

    UploadedBufferResult create_and_upload_buffer(daxa::Device & device, void const * data,
                                                  daxa::BufferInfo const & buffer_info)
    {
        auto buffer_id = device.create_buffer(buffer_info);

        std::string name = "task_" + std::string(buffer_info.name.data(), buffer_info.name.size());
        return create_and_upload_buffer(device, buffer_id, data, buffer_info.size, name);
    }

} // namespace sylva
