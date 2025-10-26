#pragma once

#include <daxa/daxa.hpp>
#include <daxa/utils/task_graph.hpp>

namespace sylva
{

    // TODO(lstallknecht): Make upload TaskGraph reusable for multiple uploads
    // (currently we create a new TaskGraph per upload which is inefficient —
    // instead we should build once and reuse with parameterised data and buffers).

    inline daxa::TaskBuffer upload_buffer(daxa::Device device, daxa::BufferId buffer_id,
                                          void * data, daxa::usize size,
                                          std::string name = "upload_buffer_task_buffer")
    {
        auto task_buffer = daxa::TaskBuffer({
            .initial_buffers = {.buffers = std::span{&buffer_id, 1}},
            .name = name,
        });
        auto upload_tg = daxa::TaskGraph({
            .device = device,
            .name = "upload_buffer_task_graph",
        });

        upload_tg.use_persistent_buffer(task_buffer);
        upload_tg.add_task(
            daxa::Task::Transfer("upload_buffer_task")
                .writes(daxa::TaskStage::TRANSFER, task_buffer)
                .executes(
                    [&](daxa::TaskInterface ti)
                    {
                        daxa::BufferId staging_buffer_id = ti.device.create_buffer({
                            .size = size,
                            .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE,
                            .name = "staging_buffer",
                        });
                        ti.recorder.destroy_buffer_deferred(staging_buffer_id);

                        auto * mapped =
                            ti.device.buffer_host_address_as<void *>(staging_buffer_id).value();
                        std::memcpy(mapped, data, size);

                        ti.recorder.copy_buffer_to_buffer({
                            .src_buffer = staging_buffer_id,
                            .dst_buffer = buffer_id,
                            .size = size,
                        });
                    }));
        upload_tg.submit({});
        upload_tg.complete({});
        upload_tg.execute({});

        return task_buffer;
    }

    inline daxa::TaskBuffer upload_buffer(daxa::Device device, void * data,
                                          daxa::BufferInfo buffer_info)
    {
        daxa::BufferId buffer_id = device.create_buffer(buffer_info);

        std::string name = "task_" + std::string(buffer_info.name.data(), buffer_info.name.size());
        return upload_buffer(device, buffer_id, data, buffer_info.size, std::move(name));
    }

} // namespace sylva
